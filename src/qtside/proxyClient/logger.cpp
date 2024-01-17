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

}
void Logger::addResponse(QString response)
{
    responsesHistory.push_back(response);
    MainWindow::getInstance()->setResponseText(response);
}
