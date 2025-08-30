#include "mainwindow.h"
#include "logger.h"
#include "QWidget"
#include "QVBoxLayout"
#include "QPushButton"
#include "QLabel"
#include "QTabBar"
#include "QString"
#include "QStackedWidget"
#include "QFile"
#include "QErrorMessage"
#include "QCoreApplication"
#include "windowbuilder.h"

int windowThreshold = 1;
std::vector<MainWindow*> windows;

MainWindow* MainWindow::openNewWindow(bool classic) {
    MainWindow* w = new MainWindow(classic);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->setFixedSize(getWindowSize(classic));
    w->setWindowTitle("About This PC");
    w->show();

    QIcon appicon(QString("%1/.AboutThisPC/AboutThisPC.png").arg(std::getenv("HOME")));
    Logger::print(QString("Setting app icon to: %1").arg(appicon.name()));
    w->setWindowIcon(appicon);

    addWindow(w);
    Logger::print(QString("Added window! (classic: %1)").arg(classic));
    return w;
}

void MainWindow::addWindow(MainWindow* w) {
    windows.push_back(w);
}

void MainWindow::removeMostRecentWindow() {
    MainWindow* w = windows.back();
    Logger::print(QString("Closing window..."));
    w->close();
    delete w;
    windows.pop_back();
}

void MainWindow::closeAllWindows() {
    for (int i = 0; i < windows.size(); i++) {
        MainWindow* w = *(windows.rbegin() + i);
        Logger::print(QString("Closing window %1...").arg(i));
        w->close();
        delete w;
    }

    Logger::print(QString("Closed %1 windows!").arg(windows.size()));
    windows.clear();
}

void MainWindow::processParent(QWidget* parent) {
    QList<QLabel*> labels = parent->findChildren<QLabel*>();

    for (int i = 0; i < labels.size(); i++) {
        QLabel* label = labels[i];
        label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    }
}

MainWindow::MainWindow(bool classic, QWidget *parent) : QMainWindow(parent) {
    Logger::print("Building window...");
    setCentralWidget(WindowBuilder::build(this, classic));
}

QSize MainWindow::getWindowSize(bool classic) {
    if (classic) {
        return QSize(350, 500); // Vertical
    } else {
        return QSize(600, 300); // Horizontal
    }
}

MainWindow::~MainWindow() {}
