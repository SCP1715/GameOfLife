#ifndef WORKER_H
#define WORKER_H

#include <amp.h>
#include <atomic>
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
        QVector<QVector<short>> tmp;
        QVector<QVector<short>> tmp2;
        {
            QMutexLocker lock(&locker);
            tmp = (*table);
        }
        tmp2 = tmp;
        bool num = true;
        while (m_running) {
            QVector<QVector<short>> *buff1;
            QVector<QVector<short>> *buff2;

            if (num) {
                buff1 = &tmp;
                buff2 = &tmp2;
            } else {
                buff2 = &tmp;
                buff1 = &tmp2;
            }
            for (int i = 1; i < (*buff2).size() - 1; i++)
                for (int j = 1; j < (*buff2)[i].size() - 1; j++) {
                    int c = (*buff2)[i - 1][j - 1] + (*buff2)[i - 1][j] + (*buff2)[i - 1][j + 1]
                            + (*buff2)[i][j - 1] + (*buff2)[i][j + 1] + (*buff2)[i + 1][j - 1]
                            + (*buff2)[i + 1][j] + (*buff2)[i + 1][j + 1];
                    if (c > 3)
                        c = 0;
                    else if (c == 3)
                        c = 1;
                    else if (c < 2)
                        c = 0;
                    else if (c == 2)
                        c = (*buff2)[i][j];
                    (*buff1)[i][j] = c;
                }
            {
                QMutexLocker lock(&locker);
                (*table) = (*buff1);
            }
            num = !num;
            emit progress();
        }
    }

    void stop() { m_running = false; }
    void start() { m_running = true; }

signals:
    void progress();
    void finished();

private:
    std::atomic<bool> m_running{false};
    QVector<QVector<short>> *table;
};

#endif // WORKER_H
