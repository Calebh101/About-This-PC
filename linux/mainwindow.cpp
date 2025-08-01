#include "mainwindow.h"
#include "logger.h"
#include "QWidget"
#include "QVBoxLayout"
#include "QPushButton"
#include "QLabel"
#include "QTabBar"
#include "QString"
#include "tabpage.h"
#include "QStackedWidget"
#include "QFile"
#include "QErrorMessage"
#include "QCoreApplication"
#include "displays.h"
#include "classicpage.h"

MainWindow::MainWindow(bool classic, QWidget *parent) : QMainWindow(parent) {
    if (classic) {
        Logger::print("Loading in classic mode...");
        setCentralWidget(ClassicPage::page(this));
    } else {
        QWidget *central = new QWidget(this);
        QVBoxLayout *mainLayout = new QVBoxLayout(central);
        mainLayout->setContentsMargins(0, 0, 0, 0);

        QTabBar *tabBar = new QTabBar();
        QHBoxLayout *centerLayout = new QHBoxLayout();
        QTabWidget *tabWidget = new QTabWidget(tabBar);

        tabWidget->addTab(LocalTabPage::overview(this), "Overview");
        tabWidget->addTab(Displays::page(this), "Displays");

        mainLayout->addWidget(tabWidget);
        mainLayout->addLayout(centerLayout);
        setCentralWidget(central);
    }
}

MainWindow::~MainWindow() {}
