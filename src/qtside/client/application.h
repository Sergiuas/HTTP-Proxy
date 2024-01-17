#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>

class Application
{
public:
    Application(Application& other) = delete;
    void operator=(const Application&) = delete;
    static Application* getInstance();

private:
    bool interceptActive;
    Application();
};

#endif // APPLICATION_H
