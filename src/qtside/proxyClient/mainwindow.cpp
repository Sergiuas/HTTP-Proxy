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
    ui->requestsList->setColumnCount(2);
    QStringList headerLabels;
    headerLabels << "Type of Request" << "Host";
    ui->requestsList->setHorizontalHeaderLabels(headerLabels);
    ui->requestsList->setColumnWidth(0,200);
    ui->requestsList->setColumnWidth(1,900);
    ui->menuTabMenu->setCurrentIndex(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addListElement(QString type, QString hostname)
{
    int newRow = ui->requestsList->rowCount();
    ui->requestsList->insertRow(newRow);
    QTableWidgetItem *typeCol = new QTableWidgetItem(type);
    ui->requestsList->setItem(newRow, 0, typeCol);
    QTableWidgetItem *hostnameCol = new QTableWidgetItem(hostname);
    ui->requestsList->setItem(newRow,1,hostnameCol);

}

void MainWindow::addBlockedElement(QString site)
{
    ui->blockedSitesList->addItem(new QListWidgetItem(site));
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
    //Intercepter::GetInstance()->setResponse(ui->responseText->toPlainText());
}


void MainWindow::on_dropBtn_clicked()
{
    Intercepter::GetInstance()->setDrop(true);
}

