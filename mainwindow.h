#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ThreadController.h"
#include <amp.h>
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
    void paintEvent(QPaintEvent *event);
    ThreadController *m_controller;
    Concurrency::array_view<short, 2> table_gpu();
    QVector<QVector<short>> table;
    std::atomic<bool> changed;

private slots:
    void slotGenerate();
    void slotStartClicked();
    void slotStopClicked();
    void slotProgress();
};

#endif // MAINWINDOW_H
