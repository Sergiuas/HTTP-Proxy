#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include "mainwindow.h"

class Logger
{
public:
    Logger(Logger& other) = delete;
    void operator=(const Logger&) = delete;
    static Logger* GetInstance();
    void addRequest(QString request);
    void addResponse(QString response);

private:
    QVector <QString> requestsHistory;
    QVector <QString> responsesHistory;
    static Logger* _logger;
    Logger();
};

#endif // LOGGER_H
