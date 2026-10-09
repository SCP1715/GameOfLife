#include "../Worker.h"

#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);
    QVector<QVector<short>> board(5, QVector<short>(5, 0));
    board[2][1] = 1;
    board[2][2] = 1;
    board[2][3] = 1;

    Worker worker(&board);
    int generations = 0;
    QString error;
    QImage lastFrame;
    QObject::connect(&worker, &Worker::progress, &application, [&](const QImage &frame) {
        ++generations;
        lastFrame = frame;
        if (generations == 2) {
            worker.stop();
        }
    }, Qt::DirectConnection);
    QObject::connect(&worker, &Worker::failed, &application, [&](const QString &message) {
        error = message;
    }, Qt::DirectConnection);

    worker.start();
    worker.process();

    if (!error.isEmpty() || generations != 2) {
        return 1;
    }

    const QVector<QVector<short>> expected = {
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 1, 1, 1, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
    };
    if (lastFrame.size() != QSize(5, 5) || lastFrame.format() != QImage::Format_Grayscale8
        || lastFrame.bytesPerLine() != 8) {
        return 2;
    }
    if (board.size() != expected.size()) {
        return 2;
    }
    for (int y = 0; y < expected.size(); ++y) {
        if (board[y].size() != expected[y].size()) {
            return 2;
        }
        for (int x = 0; x < expected[y].size(); ++x) {
            if (board[y][x] != expected[y][x]) {
                return 2;
            }
            const QRgb expectedPixel = expected[y][x] ? qRgb(0, 0, 0) : qRgb(255, 255, 255);
            if (lastFrame.pixel(x, y) != expectedPixel) {
                return 2;
            }
        }
    }
    return 0;
}
