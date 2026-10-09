#ifndef WORKER_H
#define WORKER_H

#include <amp.h>
#include <atomic>
#include <exception>
#include <stdexcept>
#include <vector>
#include <QMutex>
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

            std::vector<int> current(height * width);
            std::vector<int> next(height * width, 0);
            for (int y = 0; y < height; ++y) {
                if (initial[y].size() != width) {
                    throw std::runtime_error("The field rows must have equal width.");
                }
                for (int x = 0; x < width; ++x) {
                    current[y * width + x] = initial[y][x] != 0;
                }
            }

            const Concurrency::accelerator device = selectGpu();
            const Concurrency::accelerator_view view = device.default_view;
            const Concurrency::extent<2> extent(height, width);

            while (m_running) {
                {
                    Concurrency::array_view<int, 2> source(height, width, current);
                    Concurrency::array_view<int, 2> destination(height, width, next);
                    Concurrency::parallel_for_each(
                        view,
                        extent,
                        [=](Concurrency::index<2> cell) restrict(amp) {
                            const int y = cell[0];
                            const int x = cell[1];
                            if (y == 0 || x == 0 || y == height - 1 || x == width - 1) {
                                destination[cell] = 0;
                            } else {
                                const int neighbors = source(y - 1, x - 1) + source(y - 1, x)
                                                      + source(y - 1, x + 1) + source(y, x - 1)
                                                      + source(y, x + 1) + source(y + 1, x - 1)
                                                      + source(y + 1, x) + source(y + 1, x + 1);
                                const int alive = source[cell];
                                destination[cell] = (neighbors == 3 || (alive && neighbors == 2)) ? 1 : 0;
                            }
                        });
                    destination.synchronize();
                }

                current.swap(next);
                {
                    QMutexLocker lock(&locker);
                    for (int y = 0; y < height; ++y) {
                        for (int x = 0; x < width; ++x) {
                            (*table)[y][x] = current[y * width + x];
                        }
                    }
                }
                emit progress();
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
    void progress();
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
