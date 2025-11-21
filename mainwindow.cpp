#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QPainter>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_controller(new ThreadController(&table, this))
{
    ui->setupUi(this);

    ui->pushButton->setText("Generate");
    ui->pushButton_2->setText("Start");
    ui->pushButton_3->setText("Stop");
    ui->pushButton_3->setEnabled(false);

    connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::slotGenerate);
    connect(ui->pushButton_2, &QPushButton::clicked, this, &MainWindow::slotStartClicked);
    connect(ui->pushButton_3, &QPushButton::clicked, this, &MainWindow::slotStopClicked);
    connect(m_controller, &ThreadController::progress, this, &MainWindow::slotProgress);

    for (int i = 0; i < this->height() - 100; i++) {
        table.append(QVector<short>());
        for (int j = 0; j < this->width() - 100; j++) {
            table[i].append(0);
        }
    }

    changed = false;

    update();
}

void MainWindow::slotGenerate()
{
    QMutexLocker lock(&m_controller->m_worker->locker);
    for (int i = 0; i < table.size(); i++)
        for (int j = 0; j < table[i].size(); j++) {
            if (i == 0 || i == table.size() - 1)
                table[i][j] = 0;
            else if (j == 0 || j == table[i].size() - 1)
                table[i][j] = 0;
            else
                table[i][j] = !(bool) (rand() & 3);
        }
    lock.unlock();
    changed = true;
    update();
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    if (changed) {
        changed = false;
        QMutexLocker lock(&m_controller->m_worker->locker);
        QPainter painter(this);
        QImage image(table[0].size(), table.size(), QImage::Format_RGB32);
        for (int i = 0; i < table.size(); i++) {
            for (int j = 0; j < table[i].size(); j++) {
                image.setPixel(j, i, table[i][j] ? qRgb(0, 0, 0) : qRgb(255, 255, 255));
            }
        }
        painter.drawImage(50, 10, image);
    }
}

MainWindow::~MainWindow()
{
    delete m_controller;
    delete ui;
}

void MainWindow::slotStartClicked()
{
    m_controller->start();
    ui->pushButton->setEnabled(false);
    ui->pushButton_2->setEnabled(false);
    ui->pushButton_3->setEnabled(true);
}

void MainWindow::slotStopClicked()
{
    m_controller->stop();
    ui->pushButton->setEnabled(true);
    ui->pushButton_2->setEnabled(true);
    ui->pushButton_3->setEnabled(false);
}

void MainWindow::slotProgress()
{
    changed = true;
    update();
}
