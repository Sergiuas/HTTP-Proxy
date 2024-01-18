#ifndef INTERCEPTER_H
#define INTERCEPTER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QCoreApplication>
#include <QRegularExpression>
#include <mutex>
#include <QThread>
#include <QFile>
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
    void setDrop(bool value) { drop=value; }
    void setResponse(QString response){ this->response=response.toLatin1(); }
    void setRequest(QString request) { this->request=request.toLatin1();}
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
        //serverSocket->connectToHost("10.10.24.28", 8081);
        //serverSocket->connectToHost("192.168.56.56", 8081);
        serverSocket->connectToHost("192.168.43.119", 8081);
        if (!serverSocket->waitForConnected()) {
            qWarning() << "Failed to connect to the server";
        }
        QFile file(":/files/blocked.txt");

        // Open the file in read-only mode
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);

            // Read and output the file content line by line
            while (!in.atEnd()) {
                blockedList.append(in.readLine());
                MainWindow::getInstance()->addBlockedElement(blockedList.last());
            }

            // Close the file when done
            file.close();
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
    bool drop=false;
    QVector <QString> endConnectionCodes={"HTTP/1.1 200","HTTP/1.1 301","HTTP/1.1 302","HTTP/1.1 304","HTTP/1.1 400","HTTP/1.1 401","HTTP/1.1 403","HTTP/1.1 500"};
    QVector <QString> blockedList;
    QByteArray response="";
    QByteArray request="";

    QTcpSocket* serverSocket;

    void handleClientRequest(QTcpSocket *clientSocket);
};


#endif // INTERCEPTER_H
