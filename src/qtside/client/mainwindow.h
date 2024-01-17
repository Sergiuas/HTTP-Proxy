#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QMainWindow>
#include <QStackedWidget>
#include <QFile>
#include <QVBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QStackedWidget>
#include <QtUiTools/QUiLoader>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    Ui::MainWindow* getUi();
private slots:
    void file_open() {
        // Show the first widget in the stacked widget
        //centralWidget->setCurrentIndex(0);
    }

    void file_save() {
        // Show the second widget in the stacked widget
        //centralWidget->setCurrentIndex(1);
    }

    void file_save_as() {
        // Show the third widget in the stacked widget
        //centralWidget->setCurrentIndex(2);
    }

    void on_actionAbout_triggered();

    void on_actionIntercept_triggered();

    void on_interceptToggler_toggled(bool checked);

    void on_sendResponseBtn_clicked();


private:
    Ui::MainWindow *ui;
    void initializeComponents();


};
#endif // MAINWINDOW_H
