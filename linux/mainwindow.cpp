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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
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

MainWindow::~MainWindow() {}
