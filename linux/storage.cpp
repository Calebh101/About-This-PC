#include "storage.h"
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QGridLayout>
#include "global.h"
#include <QProgressBar>
#include "logger.h"
#include <QScrollArea>

Storage::Storage() {}

QWidget* Storage::page(QWidget* parent) {
    QWidget* page = new QWidget;
    json data = Global::getAllDisks();

    if (data["status"] == false || data["drives"].empty()) {
        QVBoxLayout* layout = new QVBoxLayout;
        QLabel* titleLabel = new QLabel;
        QFont titleFont;
        titleFont.setPointSize(16);
        titleFont.setBold(true);
        titleLabel->setFont(titleFont);
        titleLabel->setText("Whoops!");
        titleLabel->setAlignment(Qt::AlignCenter);

        QLabel* messageLabel = new QLabel;
        messageLabel->setTextFormat(Qt::RichText);
        messageLabel->setText(QString("We couldn't %1 your drives.").arg(data["status"] == false ? "load" : "find"));
        messageLabel->setAlignment(Qt::AlignCenter);

        layout->addWidget(titleLabel);
        layout->addWidget(messageLabel);
        page->setLayout(layout);
        return page;
    }

    std::vector<json> drives = data["drives"];
    QWidget* container = new QWidget;
    QGridLayout* layout = new QGridLayout(container);
    QScrollArea* scrollable = new QScrollArea;
    QVBoxLayout* pageLayout = new QVBoxLayout(page);

    for (int i = 0; i < drives.size(); i++) {
        QWidget* container = new QWidget;
        container->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        QVBoxLayout* sublayout = new QVBoxLayout;
        sublayout->setAlignment(Qt::AlignCenter);
        json drive = drives[i];
        if (drive["status"] == false) continue;
        json data = drive["data"];
        if (data["status"] == false) continue;
        QString name = QString::fromStdString(drive["path"].get<std::string>());
        Logger::print(QString("Found drive: %1").arg(name));

        long long total = data["bytes"];
        long long used = data["used"];
        long long percentUsed = (used * 100) / total;

        double totalG = static_cast<double>(total) / (1024 * 1024 * 1024);
        double usedG = static_cast<double>(used) / (1024 * 1024 * 1024);

        QProgressBar* bar = new QProgressBar();
        bar->setRange(0, 100);
        bar->setValue(percentUsed);

        QLabel* title = new QLabel();
        QFont titleFont;
        title->setAlignment(Qt::AlignCenter);
        title->setText(drive["startup"] ? QString("%1 - Startup Disk").arg(name) : name);
        titleFont.setPointSize(12);
        title->setFont(titleFont);

        QLabel* subtitle = new QLabel();
        QFont subtitleFont;
        subtitle->setAlignment(Qt::AlignCenter);
        subtitle->setText(QString("%1 GiB / %2 GiB (%3%) - %4").arg(Global::trimDecimal(usedG)).arg(Global::trimDecimal(totalG)).arg(std::to_string(percentUsed)).arg(data["external"] ? "External" : "Internal"));
        subtitleFont.setPointSize(9);
        subtitle->setFont(subtitleFont);

        sublayout->addWidget(title);
        sublayout->addWidget(subtitle);
        sublayout->addWidget(bar);

        container->setLayout(sublayout);
        layout->addWidget(container, i / 2, i % 2);
    }

    scrollable->setWidgetResizable(true);
    scrollable->setWidget(container);
    scrollable->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    pageLayout->addWidget(scrollable);
    page->setLayout(pageLayout);
    return page;
}
