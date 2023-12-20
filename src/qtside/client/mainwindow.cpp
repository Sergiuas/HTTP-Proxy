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

    centralWidget = new QStackedWidget(this);

    QFile file(":/files/about.ui");  // Adjust the file path accordingly
    file.open(QFile::ReadOnly);
    QWidget *about_widget = loadUi(&file, this);
    file.close();
    centralWidget->addWidget(about_widget);

    QFile file(":/files/intercept.ui");  // Adjust the file path accordingly
    file.open(QFile::ReadOnly);
    QWidget *intercept_widget = loadUi(&file, this);
    file.close();
    centralWidget->addWidget(intercept_widget);

     setCentralWidget(centralWidget);

}

MainWindow::~MainWindow()
{
    delete ui;
}




void MainWindow::on_actionAbout_triggered()
{
    centralWidget->setCurrentIndex(0);
}


void MainWindow::on_actionIntercept_triggered()
{
    centralWidget->setCurrentWidget(1);
}

