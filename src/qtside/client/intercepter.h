#ifndef INTERCEPTER_H
#define INTERCEPTER_H
#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <mutex>

class Intercepter : public QTcpServer
{
public:
    Intercepter(Intercepter& other) = delete;
    void operator=(const Intercepter&) = delete;
    static Intercepter* GetInstance();
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
        serverSocket->connectToHost("10.10.24.28", 8080);
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
    QTcpSocket* serverSocket;

    void handleAnswer(QTcpSocket* socket)
        {

        }

     void handleClientRequest(QTcpSocket *clientSocket) {
         QByteArray requestData = clientSocket->readAll();
         qDebug()<<requestData;
         QByteArray responseData;


         serverSocket->write(requestData);

           serverSocket->waitForBytesWritten();

             // // Read the response from the other server
             responseData = serverSocket->readAll();
            qDebug() << "Response from other server:" << responseData;
            // qWarning() << "Server socket is not connected";
             responseData = "HTTP/1.1 500 Internal Server Error\r\n\r\nServer connection error";
         // Send response back to the browser
         clientSocket->write(responseData);
         clientSocket->waitForBytesWritten();
         clientSocket->disconnectFromHost();
     }
};

#endif // INTERCEPTER_H
