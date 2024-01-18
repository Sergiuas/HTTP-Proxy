#include "logger.h"

Logger::Logger() {}

Logger* Logger::_logger;

Logger* Logger::GetInstance()
{
    if(_logger==nullptr) _logger=new Logger();
    return _logger;

}

void Logger::addRequest(QString request)
{
    requestsHistory.push_back(request);
    MainWindow::getInstance()->setRequestText(request);
    MainWindow::getInstance()->setResponseText("");
    QString type=request.split(" ")[0];
    if(request.split("\r\n").count()>1)
    {
        QString hostname;
        if(request.split("\r\n")[1].split(" ").count()>1)
            hostname=request.split("\r\n")[1].split(" ")[1];
        MainWindow::getInstance()->addListElement(type,hostname);
    }

}
void Logger::addResponse(QString response)
{
    responsesHistory.push_back(response);
    MainWindow::getInstance()->setResponseText(response);
}

void Logger::setLastRequest(QString request)
{
    if(requestsHistory.empty()) return;
    requestsHistory.last()=request;
}

void Logger::setLastResponse(QString response)
{
    if(requestsHistory.empty()) return; //if there is no requests it means that there aren't any responses
    if(requestsHistory.count()>responsesHistory.count())
        responsesHistory.push_back(response);
    else responsesHistory.last()=response;
}
