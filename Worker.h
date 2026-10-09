#ifndef WORKER_H
#define WORKER_H

#include <amp.h>
#include <atomic>
#include <exception>
#include <stdexcept>
#include <utility>
#include <vector>
#include <QMutex>
#include <QImage>
#include <QObject>
#include <QThread>

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QVector<QVector<short>> *_table, QObject *parent = nullptr)
        : QObject(parent)
        , table(_table)
    {}
    QMutex locker;

public slots:

    void process()
    {
        try {
            QVector<QVector<short>> initial;
            {
                QMutexLocker lock(&locker);
                initial = *table;
            }

            const int height = initial.size();
            const int width = height == 0 ? 0 : initial[0].size();
            if (height < 3 || width < 3) {
                throw std::runtime_error("The field must be at least 3x3.");
            }

            const int paddedWidth = (width + 3) & ~3;
            const int packedWidth = paddedWidth / 4;
            std::vector<unsigned int> current(height * packedWidth, 0xFFFFFFFFu);
            constexpr unsigned int deadPixel = 255u;
            constexpr unsigned int alivePixel = 0u;
            for (int y = 0; y < height; ++y) {
                if (initial[y].size() != width) {
                    throw std::runtime_error("The field rows must have equal width.");
                }
                for (int x = 0; x < width; ++x) {
                    if (initial[y][x] != 0) {
                        const int shift = (x % 4) * 8;
                        current[y * packedWidth + x / 4] &= ~(0xFFu << shift);
                    }
                }
            }

            const Concurrency::accelerator device = selectGpu();
            const Concurrency::accelerator_view view = device.default_view;
            const Concurrency::extent<2> extent(height, packedWidth);
            Concurrency::array<unsigned int, 2> firstBuffer(extent, view);
            Concurrency::array<unsigned int, 2> secondBuffer(extent, view);
            Concurrency::copy(current.begin(), current.end(), firstBuffer);

            Concurrency::array<unsigned int, 2> *sourceBuffer = &firstBuffer;
            Concurrency::array<unsigned int, 2> *destinationBuffer = &secondBuffer;
            QImage lastFrame;

            while (m_running) {
                QImage frame(width, height, QImage::Format_Grayscale8);
                if (frame.isNull() || frame.bytesPerLine() != paddedWidth) {
                    throw std::runtime_error("Could not allocate an aligned grayscale frame.");
                }

                {
                    Concurrency::array_view<const unsigned int, 2> source(*sourceBuffer);
                    Concurrency::array_view<unsigned int, 2> destination(*destinationBuffer);
                    Concurrency::parallel_for_each(
                        view,
                        extent,
                        [=](Concurrency::index<2> cell) restrict(amp) {
                            const int y = cell[0];
                            unsigned int outputWord = 0;
                            for (int lane = 0; lane < 4; ++lane) {
                                const int x = cell[1] * 4 + lane;
                                unsigned int pixel = deadPixel;
                                if (y > 0 && y < height - 1 && x > 0 && x < width - 1) {
                                    int neighbors = 0;
                                    for (int dy = -1; dy <= 1; ++dy) {
                                        for (int dx = -1; dx <= 1; ++dx) {
                                            if (dx == 0 && dy == 0) {
                                                continue;
                                            }
                                            const int column = x + dx;
                                            const unsigned int word = source(y + dy, column / 4);
                                            const int shift = (column % 4) * 8;
                                            neighbors += ((word >> shift) & 0xFFu) == alivePixel;
                                        }
                                    }
                                    const unsigned int word = source[cell];
                                    const int shift = lane * 8;
                                    const bool alive = ((word >> shift) & 0xFFu) == alivePixel;
                                    pixel = (neighbors == 3 || (alive && neighbors == 2))
                                                ? alivePixel
                                                : deadPixel;
                                }
                                outputWord |= pixel << (lane * 8);
                            }
                            destination[cell] = outputWord;
                        });
                }

                Concurrency::copy(*destinationBuffer, reinterpret_cast<unsigned int *>(frame.bits()));
                lastFrame = frame;
                emit progress(frame);

                std::swap(sourceBuffer, destinationBuffer);
            }

            if (!lastFrame.isNull()) {
                QMutexLocker lock(&locker);
                for (int y = 0; y < height; ++y) {
                    const auto *row = lastFrame.constScanLine(y);
                    for (int x = 0; x < width; ++x) {
                        (*table)[y][x] = row[x] == alivePixel;
                    }
                }
            }
        } catch (const std::exception &error) {
            m_running = false;
            emit failed(QString::fromLatin1(error.what()));
        } catch (...) {
            m_running = false;
            emit failed(QStringLiteral("Unknown GPU calculation error."));
        }
        emit finished();
    }

    void stop() { m_running = false; }
    void start() { m_running = true; }

signals:
    void progress(const QImage &frame);
    void failed(const QString &message);
    void finished();

private:
    static Concurrency::accelerator selectGpu()
    {
        const std::vector<Concurrency::accelerator> devices = Concurrency::accelerator::get_all();
        for (const Concurrency::accelerator &device : devices) {
            if (!device.is_emulated && device.device_path != Concurrency::accelerator::direct3d_ref
                && device.device_path != Concurrency::accelerator::direct3d_warp
                && device.device_path != Concurrency::accelerator::cpu_accelerator) {
                return device;
            }
        }
        throw std::runtime_error("No hardware GPU compatible with C++ AMP was found.");
    }

    std::atomic<bool> m_running{false};
    QVector<QVector<short>> *table;
};

#endif // WORKER_H
