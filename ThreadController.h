#ifndef THREADCONTROLLER_H
#define THREADCONTROLLER_H

#include "Worker.h"
#include <QObject>
#include <QThread>

class ThreadController : public QObject
{
    Q_OBJECT
public:
    explicit ThreadController(QVector<QVector<short>> *_table, QObject *parent = nullptr)
        : QObject(parent)
        , m_worker(new Worker(_table))
        , m_thread(new QThread())
    {
        m_worker->moveToThread(m_thread);

        connect(m_thread, &QThread::started, m_worker, &Worker::process);
        connect(m_worker, &Worker::progress, this, &ThreadController::progress);
        connect(m_worker, &Worker::failed, this, &ThreadController::failed);
        connect(m_worker, &Worker::finished, m_thread, &QThread::quit);
    }

    ~ThreadController()
    {
        if (m_thread->isRunning()) {
            stop();
            m_thread->quit();
            m_thread->wait();
        }
        delete m_thread;
        delete m_worker;
    }

    Worker *m_worker;

public slots:
    void start()
    {
        if (m_worker)
            m_worker->start();
        if (!m_thread->isRunning()) {
            m_thread->start();
        }
    }

    void stop()
    {
        if (m_worker)
            m_worker->stop();
        m_thread->quit();
    }

signals:
    void failed(const QString &message);
    void progress(); // Прокси-сигнал от worker
    void finished(); // Прокси-сигнал завершения

private:
    QThread *m_thread;
};

#endif // THREADCONTROLLER_H
