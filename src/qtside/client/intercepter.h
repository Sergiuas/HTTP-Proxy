#ifndef INTERCEPTER_H
#define INTERCEPTER_H
#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <mutex>
#include <QThread>

class Intercepter : public QTcpServer
{
public:
    Intercepter(Intercepter& other) = delete;
    void operator=(const Intercepter&) = delete;
    static Intercepter* GetInstance(Ui::MainWindow * ui);
protected:
    void incomingConnection(qintptr socketDescriptor) override {
        QTcpSocket *clientSocket = new QTcpSocket(this);
        clientSocket->setSocketDescriptor(socketDescriptor);

        connect(clientSocket, &QTcpSocket::readyRead, this, [this, clientSocket]() {
            handleClientRequest(clientSocket);
        });
    }

private:
    Intercepter(Ui::MainWindow * ui,QObject *parent = nullptr): QTcpServer(parent){
        this->ui=ui;
        this->serverSocket = new QTcpSocket(this);
<<<<<<< Updated upstream
        //serverSocket->connectToHost("10.10.24.28", 8081);
        serverSocket->connectToHost("192.168.56.56", 8080);
=======
        serverSocket->connectToHost("10.10.24.28", 8081);
>>>>>>> Stashed changes
        if (!serverSocket->waitForConnected()) {
            qWarning() << "Failed to connect to the server";
        }


    }
    ~Intercepter(){
        serverSocket->disconnectFromHost();
        serverSocket->waitForDisconnected();
        delete serverSocket;
    }
    static Intercepter* intercepter_;
    static std::mutex mutex_;
    Ui::MainWindow * ui;
    QTcpSocket* serverSocket;

    void handleAnswer(QTcpSocket* socket)
        {

        }

     void handleClientRequest(QTcpSocket *clientSocket) {
         QByteArray requestData = clientSocket->readAll();
         qDebug()<<requestData;
         QByteArray responseData;

        // requestData = "GET / HTTP/1.1\r\nHost: www.google.com\r\n\r\n";

         serverSocket->write(requestData);

         serverSocket->waitForBytesWritten();

<<<<<<< Updated upstream
         // // Read the response from the other server
         serverSocket->waitForReadyRead();
         responseData = serverSocket->readAll();
         qDebug() << "Response from other server:" << responseData;
         if(ui->interceptToggler->isChecked()) {
             ui->responseBody->append(responseData);
         }
         while(ui->interceptToggler->isChecked()){
            QCoreApplication::processEvents();
         }
         // qWarning() << "Server socket is not connected";
         //responseData = "HTTP/1.1 500 Internal Server Error\r\n\r\nServer connection error";
         //verify if intercept is on
=======
             // // Read the response from the other server
             responseData = serverSocket->readAll();
            qDebug() << "Response from other server:" << responseData;
            // qWarning() << "Server socket is not connected";
            // responseData = "HTTP/1.1 500 Internal Server Error\r\n\r\nServer connection error";
>>>>>>> Stashed changes
         // Send response back to the browser
         clientSocket->write(responseData);
         clientSocket->waitForBytesWritten();
         clientSocket->disconnectFromHost();
     }
};

#endif // INTERCEPTER_H
