#include <QObject>
#include <QTcpSocket>

class Client : public QObject
{
    Q_OBJECT

public:
    explicit Client(QObject *parent = nullptr);

public slots:
    void connectToServer(const QHostAddress &address, quint16 port);
    void sendMessage(const QString &text);
    void disconnectFromHost();
private slots:
    void onReadyRead(){
        QByteArray jsonData;
        QDataStream socketStream(m_clientSocket);
        for (;;) {
            // we start a transaction so we can revert to the previous state in case we try to read more data than is available on the socket
            socketStream.startTransaction();
            // we try to read the JSON data 
            socketStream >> jsonData;
            if (socketStream.commitTransaction()) {
                // we successfully read some data
                // we now need to make sure it's in fact a valid JSON
                QJsonParseError parseError;
                // we try to create a json document with the data we received
                const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);
                if (parseError.error == QJsonParseError::NoError) {
                    // if the data was indeed valid JSON
                    if (jsonDoc.isObject()) // and is a JSON object
                        jsonReceived(jsonDoc.object()); // parse the JSON
                }
            } else {
                break;
            }
        }
    }
signals:
    void connected();
    void disconnected();
    void messageReceived(const QString &sender, const QString &text);
    void error(QAbstractSocket::SocketError socketError);
    void userJoined(const QString &username);
    void userLeft(const QString &username);

private:
    QTcpSocket *m_socket;
    void jsonReceived(const QJsonObject &doc){
        

    }
};

#endif // CLIENT_H


// https://wiki.qt.io/WIP-How_to_create_a_simple_chat_application