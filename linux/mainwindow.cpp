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
#include "storage.h"

std::vector<MainWindow*> windows;

MainWindow* MainWindow::openNewWindow(bool classic) {
    MainWindow* w = new MainWindow(classic);
    w->setAttribute(Qt::WA_DeleteOnClose);
    w->setFixedSize(getWindowSize(classic));
    w->show();
    addWindow(w);
    return w;
}

void MainWindow::addWindow(MainWindow* w) {
    windows.push_back(w);
}

void MainWindow::removeMostRecentWindow() {
    if (!windows.empty()) {
        windows.back()->close();
    }
}

void MainWindow::closeAllWindows() {
    for (int i = 0; i < windows.size(); i++) {
        MainWindow* w = windows[i];
        w->close();
    }
}

void processParent(QWidget* parent) {
    QList<QLabel*> labels = parent->findChildren<QLabel*>();

    for (int i = 0; i < labels.size(); i++) {
        QLabel* label = labels[i];
        label->setTextInteractionFlags(Qt::TextSelectableByMouse);
    }
}

MainWindow::MainWindow(bool classic, QWidget *parent) : QMainWindow(parent) {
    if (classic) {
        Logger::print("Loading in classic mode...");
        processParent(this);
        setCentralWidget(ClassicPage::page(this));
    } else {
        QWidget *central = new QWidget(this);
        QVBoxLayout *mainLayout = new QVBoxLayout(central);
        mainLayout->setContentsMargins(0, 0, 0, 0);

        QTabBar *tabBar = new QTabBar();
        QHBoxLayout *centerLayout = new QHBoxLayout();
        QTabWidget *tabWidget = new QTabWidget(tabBar);
        ordered_json supportUrls = Global::getSupportUrls();

        tabWidget->addTab(LocalTabPage::overview(this), "Overview");
        tabWidget->addTab(Displays::page(this), "Displays");
        tabWidget->addTab(Storage::page(this), "Storage");
        if (!supportUrls.empty()) tabWidget->addTab(SupportPage::page(this, supportUrls), "Support");

        Logger::print("Finalizing layout...");
        tabWidget->setStyleSheet("QTabWidget::tab-bar { alignment: center; }");
        mainLayout->addWidget(tabWidget);
        mainLayout->addLayout(centerLayout);
        processParent(this);
        setCentralWidget(central);
    }
}

QSize MainWindow::getWindowSize(bool classic) {
    if (classic) {
        return QSize(350, 500); // Vertical
    } else {
        return QSize(600, 300); // Horizontal
    }
}

MainWindow::~MainWindow() {}
