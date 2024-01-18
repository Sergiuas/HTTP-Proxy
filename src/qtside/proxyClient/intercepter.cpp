#include "intercepter.h"

Intercepter* Intercepter::intercepter_=nullptr;
std::mutex Intercepter::mutex_;



Intercepter* Intercepter::GetInstance(){
    if(intercepter_==nullptr){
        intercepter_=new Intercepter();
    }
    return intercepter_;
}

void Intercepter::handleClientRequest(QTcpSocket *clientSocket)
{
    bool ok=false;
    do{
        response.clear();
        request.clear();
        request = clientSocket->readAll();
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
        qDebug() << "Response from other server:" << response;
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
