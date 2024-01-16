#include "qtclient.h"

Client::Client(QObject *parent)
{
    m_socket = new QTcpSocket(this);

    connect(m_socket, &QTcpSocket::connected, this, &Client::connected);
    connect(m_socket, &QTcpSocket::disconnected, this, &Client::disconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &Client::onReadyRead);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &Client::error);

}

void Client::sendMessage(const QString &message)
{
    if(message.isEmpty())
        return;

    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << message;

    m_socket->write(data);

}

void Client::connectToServer(const QString &serverAddress, quint16 port)
{
    m_socket->connectToHost(serverAddress, port);
}

void Client::disconnectFromHost()
{
    m_clientSocket->disconnectFromHost();
}
