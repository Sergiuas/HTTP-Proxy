#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "intercepter.h"

MainWindow* MainWindow::instance;

MainWindow* MainWindow::getInstance()
{
    if(instance==nullptr) instance=new MainWindow();
    return instance;
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::setRequestText(QString request)
{
    ui->requestText->clear();
    ui->requestText->setPlainText(request);
    ui->httpTabMenu->setCurrentIndex(0);
}
void MainWindow::setResponseText(QString response)
{
    ui->responseText->clear();
    ui->responseText->setPlainText(response);
    if(response!="") ui->httpTabMenu->setCurrentIndex(1);
}

void MainWindow::on_interceptBtn_clicked()
{
    Intercepter::GetInstance()->toggleIntercept();
    ui->interceptBtn->setChecked(!ui->interceptBtn->isChecked());
    if(Intercepter::GetInstance()->isIntercepting())
    {
        ui->interceptBtn->setStyleSheet("background-color: green;");
        ui->interceptBtn->styleSheet();
    }
    else ui->interceptBtn->setStyleSheet("");
}


void MainWindow::on_forwardBtn_clicked()
{
    Intercepter::GetInstance()->setNextAction(true);
}


void MainWindow::on_requestText_textChanged()
{
    Logger::GetInstance()->setLastRequest(ui->requestText->toPlainText());
    Intercepter::GetInstance()->setRequest(ui->requestText->toPlainText());
}


void MainWindow::on_responseText_textChanged()
{
    Logger::GetInstance()->setLastResponse(ui->responseText->toPlainText());
    Intercepter::GetInstance()->setResponse(ui->responseText->toPlainText());
}

