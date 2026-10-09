#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ThreadController.h"
#include <QImage>
#include <QMainWindow>
#include <QVector>

QT_BEGIN_NAMESPACE namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    void renderBoard();
    ThreadController *m_controller;
    QVector<QVector<short>> table;

private slots:
    void slotGenerate();
    void slotStartClicked();
    void slotStopClicked();
    void slotProgress(const QImage &frame);
    void slotWorkerFinished();
    void slotWorkerFailed(const QString &message);
};

#endif // MAINWINDOW_H
