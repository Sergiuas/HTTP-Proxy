#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    static MainWindow* getInstance();
    void setRequestText(QString request);
    void setResponseText(QString response);
private slots:
    void on_interceptBtn_clicked();

    void on_forwardBtn_clicked();

private:
    Ui::MainWindow *ui;
    static MainWindow* instance;
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

};
#endif // MAINWINDOW_H