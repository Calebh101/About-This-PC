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
#include "supportpage.h"

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
        json supportUrls = Global::getSupportUrls();

        tabWidget->addTab(LocalTabPage::overview(this), "Overview");
        tabWidget->addTab(Displays::page(this), "Displays");
        Logger::print(QString("Generating support page... (empty: %1)").arg(supportUrls.empty() ? "yes" : "no"));
        if (!supportUrls.empty()) tabWidget->addTab(SupportPage::page(this, supportUrls), "Support");

        Logger::print("Finalizing layout...");
        mainLayout->addWidget(tabWidget);
        mainLayout->addLayout(centerLayout);
        setCentralWidget(central);
    }
}

MainWindow::~MainWindow() {}
