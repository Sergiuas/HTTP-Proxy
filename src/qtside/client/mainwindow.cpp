#include "mainwindow.h"
#include "ui_mainwindow.h"

QWidget* loadUi(QFile* file, QWidget* parent = nullptr) {
    QUiLoader loader;
    QWidget* uiWidget = loader.load(file, parent);
    // file->close();
    return uiWidget;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);



}

MainWindow::~MainWindow()
{
    delete ui;
}




void MainWindow::on_actionAbout_triggered()
{
    //centralWidget->setCurrentIndex(0);
}


void MainWindow::on_actionIntercept_triggered()
{
    ui->stackedWidget->setCurrentIndex(1);
    //centralWidget->setCurrentWidget(1);
}

