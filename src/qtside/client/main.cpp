#include "mainwindow.h"
#include "intercepter.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    Intercepter* intercepter=Intercepter::GetInstance();
    if(!intercepter->listen(QHostAddress::Any, 8888)){
        qCritical() << "Failed to start server:" << intercepter->errorString();
        return 1;
    }
    return a.exec();
}
