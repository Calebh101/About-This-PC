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
    static QSize getWindowSize(bool classic = false);
    static MainWindow* openNewWindow(bool classic);
    static void addWindow(MainWindow* w);
    static void removeMostRecentWindow();
    static void closeAllWindows();
    static void processParent(QWidget* parent);
};

#endif // MAINWINDOW_H
