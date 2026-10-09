#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QImage>
#include <QMessageBox>
#include <QPixmap>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_controller(new ThreadController(&table, this))
{
    ui->setupUi(this);

    ui->generateButton->setText("Generate");
    ui->startButton->setText("Start");
    ui->stopButton->setText("Stop");
    ui->stopButton->setEnabled(false);

    connect(ui->generateButton, &QPushButton::clicked, this, &MainWindow::slotGenerate);
    connect(ui->startButton, &QPushButton::clicked, this, &MainWindow::slotStartClicked);
    connect(ui->stopButton, &QPushButton::clicked, this, &MainWindow::slotStopClicked);
    connect(m_controller, &ThreadController::progress, this, &MainWindow::slotProgress);
    connect(m_controller, &ThreadController::failed, this, &MainWindow::slotWorkerFailed);

    for (int i = 0; i < this->height() - 100; i++) {
        table.append(QVector<short>());
        for (int j = 0; j < this->width() - 100; j++) {
            table[i].append(0);
        }
    }

    renderBoard();
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
    renderBoard();
}

void MainWindow::renderBoard()
{
    QMutexLocker lock(&m_controller->m_worker->locker);
    if (table.isEmpty() || table[0].isEmpty()) {
        return;
    }

    QImage image(table[0].size(), table.size(), QImage::Format_RGB32);
    for (int i = 0; i < table.size(); i++) {
        for (int j = 0; j < table[i].size(); j++) {
            image.setPixel(j, i, table[i][j] ? qRgb(0, 0, 0) : qRgb(255, 255, 255));
        }
    }
    ui->boardLabel->setPixmap(QPixmap::fromImage(image));
}

MainWindow::~MainWindow()
{
    delete m_controller;
    delete ui;
}

void MainWindow::slotStartClicked()
{
    m_controller->start();
    ui->generateButton->setEnabled(false);
    ui->startButton->setEnabled(false);
    ui->stopButton->setEnabled(true);
}

void MainWindow::slotStopClicked()
{
    m_controller->stop();
    ui->generateButton->setEnabled(true);
    ui->startButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
}

void MainWindow::slotProgress()
{
    renderBoard();
}

void MainWindow::slotWorkerFailed(const QString &message)
{
    ui->generateButton->setEnabled(true);
    ui->startButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
    QMessageBox::critical(this, tr("Ошибка вычисления"), message);
}
