#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QResizeEvent>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(bool classic, QWidget *parent = nullptr);
    ~MainWindow();
    static MainWindow* openNewWindow(bool classic);
    static QSize getWindowSize(bool classic);
};

#endif // MAINWINDOW_H
