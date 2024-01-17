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
    initializeComponents();
}

MainWindow::~MainWindow()
{
    delete ui;
}

Ui::MainWindow* MainWindow::getUi()
{
    return ui;
}

void MainWindow::initializeComponents()
{
    ui->stackedWidget->setCurrentIndex(1);
    ui->responseBody->setVisible(false);
    ui->sendResponseBtn->setVisible(false);
}

void MainWindow::on_actionAbout_triggered()
{
    //centralWidget->setCurrentIndex(0);
}


void MainWindow::on_actionIntercept_triggered()
{
    ui->stackedWidget->setCurrentIndex(0);
    //centralWidget->setCurrentWidget(1);
}


void MainWindow::on_interceptToggler_toggled(bool checked)
{
    if(checked){
        ui->responseBody->setVisible(true);
        ui->sendResponseBtn->setVisible(true);
    }
    else {
        ui->responseBody->setVisible(false);
        ui->sendResponseBtn->setVisible(false);
    }
}

void MainWindow::on_sendResponseBtn_clicked()
{
    ui->interceptToggler->setChecked(false);
}
