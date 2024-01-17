#ifndef INTERCEPTER_H
#define INTERCEPTER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QCoreApplication>
#include <mutex>
#include <QThread>
#include "logger.h"

class Intercepter : public QTcpServer
{
public:
    Intercepter(Intercepter& other) = delete;
    void operator=(const Intercepter&) = delete;
    static Intercepter* GetInstance();

    void toggleIntercept() { activeIntercept=!activeIntercept; }
    bool isIntercepting() { return activeIntercept; }
    void setNextAction(bool value) { nextAction=value; }
protected:
    void incomingConnection(qintptr socketDescriptor) override {
        QTcpSocket *clientSocket = new QTcpSocket(this);
        clientSocket->setSocketDescriptor(socketDescriptor);

        connect(clientSocket, &QTcpSocket::readyRead, this, [this, clientSocket]() {
            handleClientRequest(clientSocket);
        });
    }

private:
    Intercepter(QObject *parent = nullptr): QTcpServer(parent){
        this->serverSocket = new QTcpSocket(this);
        serverSocket->connectToHost("10.10.24.28", 8081);
        //serverSocket->connectToHost("192.168.56.56", 8080);
        //serverSocket->connectToHost("192.168.43.119", 8081);
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
    bool activeIntercept=false;
    bool nextAction=false;

    QTcpSocket* serverSocket;

    void handleAnswer(QTcpSocket* socket)
    {

    }

    void handleClientRequest(QTcpSocket *clientSocket) {
        QByteArray requestData = clientSocket->readAll();
        qDebug()<<requestData;
        QByteArray responseData;
        if(responseData.contains("CONNECT"))
            clientSocket->disconnectFromHost();

        //set up logger;
        Logger * log=Logger::GetInstance();
        //adding request to log
        log->addRequest(requestData);

        //intercept request if intercept is active
        nextAction=false;
        if(activeIntercept){
            while(activeIntercept && !nextAction)
            {
                QCoreApplication::processEvents();
            }
            nextAction=false;
        }
        serverSocket->write(requestData);

        serverSocket->waitForBytesWritten();

        // Read the response from the other server
        serverSocket->waitForReadyRead();
        responseData += serverSocket->readAll();
        qDebug() << "Response from other server:" << responseData;

        //add response to log
        log->addResponse(responseData);

        //intercept response if intercept is active
        if(activeIntercept){
            while(activeIntercept && !nextAction)
            {
                QCoreApplication::processEvents();
            }
            nextAction=false;
        }
        // Send response back to the browser
        clientSocket->write(responseData);
        clientSocket->waitForBytesWritten();
        clientSocket->disconnectFromHost();
    }
};


#endif // INTERCEPTER_H
