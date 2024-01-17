#include "mainwindow.h"
#include "intercepter.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Intercepter *intercepter= Intercepter::GetInstance();
    MainWindow *w=MainWindow::getInstance();
    w->show();
    if(!intercepter->listen(QHostAddress::Any, 8888)){
        qCritical() << "Failed to start server:" << intercepter->errorString();
        return 1;
    }
    return a.exec();
}
