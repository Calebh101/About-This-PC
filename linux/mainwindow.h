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
};

#endif // MAINWINDOW_H
