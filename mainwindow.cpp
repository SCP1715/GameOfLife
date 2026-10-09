#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QImage>
#include <QDebug>
#include <QMessageBox>
#include <utility>

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
    connect(m_controller, &ThreadController::finished, this, &MainWindow::slotWorkerFinished);
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
        }
    renderBoard();
}

void MainWindow::renderBoard()
{
    QImage image;
    {
        QMutexLocker lock(&m_controller->m_worker->locker);
        if (table.isEmpty() || table[0].isEmpty()) {
            return;
        }

        image = QImage(table[0].size(), table.size(), QImage::Format_Grayscale8);
        for (int y = 0; y < table.size(); ++y) {
            auto *row = image.scanLine(y);
            for (int x = 0; x < table[y].size(); ++x) {
                row[x] = table[y][x] ? 0 : 255;
            }
            for (int x = table[y].size(); x < image.bytesPerLine(); ++x) {
                row[x] = 255;
            }
        }
    }
    ui->boardView->setImage(std::move(image));
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
    ui->stopButton->setEnabled(false);
}

void MainWindow::slotProgress(const QImage &frame)
{
    ui->boardView->setImage(frame);
}

void MainWindow::slotWorkerFinished()
{
    ui->generateButton->setEnabled(true);
    ui->startButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
}

void MainWindow::slotWorkerFailed(const QString &message)
{
    QMessageBox::critical(this, tr("Ошибка вычисления"), message);
}
