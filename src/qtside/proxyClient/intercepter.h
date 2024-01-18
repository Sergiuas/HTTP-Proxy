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
        QFile file("C:\\Users\\Gabi\\Desktop\\info\\pso\\HTTP-Proxy\\src\\qtside\\proxyClient\\blocked.txt");

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

    void handleClientRequest(QTcpSocket *clientSocket) {
        bool ok=false;
        do{
            response.clear();
            request.clear();
            //if (clientSocket->waitForReadyRead(100)) {
                // Data is available to read
                request = clientSocket->readAll();
                // Process the received data
            //}
            qDebug()<<"Print request:"<<request;
            for(int i=0;i<blockedList.count();i++)
                if(request.contains(blockedList[i].toStdString().c_str())){
                    clientSocket->disconnectFromHost();
                    return;
                }
            for(int i=0;i<endConnectionCodes.count();i++)
                if(request.contains(endConnectionCodes[i].toStdString().c_str())) ok=true;
            if(request.isEmpty()) {
                ok=true;
            }
            //set up logger;
            Logger * log=Logger::GetInstance();
            //adding request to log
            log->addRequest(request);

            //intercept request if intercept is active
            nextAction=false;
            drop=false;
            if(activeIntercept){
                while(activeIntercept && !nextAction && !drop)
                {
                    QCoreApplication::processEvents();
                }
                if(drop){
                    clientSocket->disconnectFromHost();
                    //delete request from history
                    return;
                }
                nextAction=false;
                drop=false;
            }
            serverSocket->write(request);

            serverSocket->waitForBytesWritten();

            // Read the response from the other server
            serverSocket->waitForReadyRead(1000);
            response.append( serverSocket->readAll());
            if(response.isEmpty()) {
                ok=true;
            }
            //while(response.endsWith("[FINISHED]")) response.append( serverSocket->readAll());
            qDebug() << "Response from other server:" << response;
            //if (response.endsWith("[FINISHED]")) response.chop(10);

            //add response to log
            log->addResponse(response);

            //intercept response if intercept is active
            if(activeIntercept){
                while(activeIntercept && !nextAction && !drop)
                {
                    QCoreApplication::processEvents();
                }
                if(drop){
                    clientSocket->disconnectFromHost();
                    //delete request from history
                    return;
                }
                nextAction=false;
                drop=false;
            }

            // Send response back to the browser
            clientSocket->write(response);
            clientSocket->waitForBytesWritten();
        }while(!ok);
        clientSocket->disconnectFromHost();
    }
};


#endif // INTERCEPTER_H
