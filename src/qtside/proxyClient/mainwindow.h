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
    void addListElement(QString type, QString hostname);
    void addBlockedElement(QString site);
private slots:
    void on_interceptBtn_clicked();

    void on_forwardBtn_clicked();

    void on_requestText_textChanged();

    void on_responseText_textChanged();

    void on_dropBtn_clicked();


    void on_blockBtn_clicked();

private:
    Ui::MainWindow *ui;
    static MainWindow* instance;
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

};
#endif // MAINWINDOW_H
