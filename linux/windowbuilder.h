#ifndef WINDOWBUILDER_H
#define WINDOWBUILDER_H

#include <qboxlayout.h>
#include <qtabbar.h>
#include <qwidget.h>
#include "mainwindow.h"
#include "logger.h"
#include "classicpage.h"
#include "tabpage.h"
#include "displays.h"
#include "storage.h"
#include "supportpage.h"

class WindowBuilder
{
public:
    static QWidget* build(MainWindow* parent, bool classic) {
        if (classic) {
            Logger::print("Loading in classic mode...");
            MainWindow::processParent(parent);
            return ClassicPage::page(parent);
        } else {
            QWidget *central = new QWidget(parent);
            QVBoxLayout *mainLayout = new QVBoxLayout(central);
            mainLayout->setContentsMargins(0, 0, 0, 0);

            QTabBar *tabBar = new QTabBar();
            QHBoxLayout *centerLayout = new QHBoxLayout();
            QTabWidget *tabWidget = new QTabWidget(tabBar);
            ordered_json supportUrls = Global::getSupportUrls();

            tabWidget->addTab(LocalTabPage::overview(parent), "Overview");
            tabWidget->addTab(Displays::page(parent), "Displays");
            tabWidget->addTab(Storage::page(parent), "Storage");
            if (!supportUrls.empty()) tabWidget->addTab(SupportPage::page(parent, supportUrls), "Support");

            Logger::print("Finalizing layout...");
            tabWidget->setStyleSheet("QTabWidget::tab-bar { alignment: center; }");
            mainLayout->addWidget(tabWidget);
            mainLayout->addLayout(centerLayout);
            MainWindow::processParent(parent);
            return central;
        }
    }
};

#endif // WINDOWBUILDER_H
