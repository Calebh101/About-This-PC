#include "mainwindow.h"
#include "classicpage.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include "global.h"
#include "json.hpp"
#include <QTableWidget>
#include "logger.h"
#include "tabpage.h"
#include <QPushButton>
#include <QProcess>
#include <QGraphicsOpacityEffect>
#include "cicon.h"
#include "themelistener.h"

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

QSpacerItem* vspacer() {
    return new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
}

QWidget* ClassicPage::hspacer() {
    QWidget *widget = new QWidget;
    widget->setFixedWidth(10);
    return widget;
}

ClassicPage::ClassicPage() {}

QWidget* ClassicPage::page(MainWindow* parent) {
    bool showPrivate = true;
    QWidget *central = new QWidget(parent);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    QHBoxLayout *mainColumnLayout = new QHBoxLayout();
    QVBoxLayout *leftLayout = new QVBoxLayout();
    QVBoxLayout *rightLayout = new QVBoxLayout();
    ordered_json results;

    json chassis = Global::getChassis();
    json cpuInfo = Global::getCPU();
    json gpuInfo = Global::getGPU();
    json osInfo = Global::getOS();
    json ramInfo = Global::getHelperData("memory");
    json serialInfo = Global::getHelperData("serial");

    std::vector<std::string> ramAttributes;
    std::string productFamily = Global::getFamily();
    std::string productName = Global::getModel();
    std::string startupDiskPath = Global::getStartupDisk();
    json startupDiskInfo = Global::getDisk(startupDiskPath);

    float speed = cpuInfo["speed"].get<float>();
    std::ostringstream oss;
    oss << std::defaultfloat << std::setprecision(2) << speed;
    std::string speedString = oss.str();

    std::string processorString = QString::fromStdString(cpuInfo["processors"].front().get<std::string>()).toStdString();
    results["Processor"] = QString("%3 %1GHz %2").arg(speedString).arg(processorString).arg(cpuInfo["arch"].get<std::string>()).toStdString();

    if (ramInfo.contains("totalString")) ramAttributes.push_back(ramInfo["totalString"]);
    if (ramInfo.contains("type")) ramAttributes.push_back(ramInfo["type"]);
    if (ramInfo.contains("form")) ramAttributes.push_back(ramInfo["form"]);
    if (ramInfo.contains("speed")) ramAttributes.push_back(ramInfo["speed"]);

    if (gpuInfo["status"] == true) {
        json gpu = gpuInfo["gpus"].front();
        int vram = gpu["vram"].get<int>();
        std::ostringstream oss;
        oss << std::defaultfloat << std::setprecision(2) << (vram / 1000.0);
        QString text = QString("%1 %2GB").arg(gpu["name"].get<std::string>()).arg(oss.str());
        results["Graphics"] = text.toStdString();
    }

    if (!ramAttributes.empty()) {
        std::ostringstream oss;

        for (size_t i = 0; i < ramAttributes.size(); ++i) {
            if (i > 0) oss << ' ';
            oss << ramAttributes[i];
        }

        results["Memory"] = oss.str();
    }

    mainLayout->addWidget(LocalTabPage::processImage(chassis["icon"], 184));
    mainLayout->addItem(vspacer());

    QLabel* familyLabel = new QLabel(QString::fromStdString(productFamily));
    QFont familyFont = familyLabel->font();
    familyFont.setPointSize(24);
    familyLabel->setFont(familyFont);
    familyLabel->setAlignment(Qt::AlignHCenter);
    mainLayout->addWidget(familyLabel);

    QLabel* productLabel = new QLabel(QString::fromStdString(productName));
    QFont productFont = productLabel->font();
    QLabel* serialValueLabel = nullptr;

    productFont.setPointSize(Global::fontSize);
    productLabel->setFont(productFont);
    productLabel->setAlignment(Qt::AlignHCenter);
    mainLayout->addWidget(productLabel);
    mainLayout->addItem(vspacer());
    if (serialInfo.contains("serial")) results["Serial"] = (showPrivate ? serialInfo["serial"] : "");

    if (startupDiskInfo["status"] == true) {
        long long bytes = startupDiskInfo["bytes"].get<long long>();
        int size = bytes / 1000.0 / 1000.0 / 1000.0;
        results["Startup Disk"] = QString("%1 %2 GB").arg(startupDiskPath).arg(std::to_string(size)).toStdString();
    } else {
        results["Startup Disk"] = startupDiskPath;
    }

    results["Kernel"] = osInfo["kernel"];

    for (auto& [key, value] : results.items()) {
        QLabel* label1 = new QLabel;
        QLabel* label2 = new QLabel;
        QString text = QString::fromStdString(value.dump());

        if (value.is_string()) text = QString::fromStdString(value.get<std::string>());
        if (key == "Serial") serialValueLabel = label2;

        label1->setTextFormat(Qt::RichText);
        label2->setTextFormat(Qt::RichText);

        label1->setText(QString("<div style='font-size: %2pt;'><span style='font-weight: %3s;'>%1</span></div>").arg(QString::fromStdString(key)).arg(std::to_string(Global::fontSize)).arg(std::to_string(Global::fontWeight)));
        label2->setText(QString("<div style='font-size: %2pt;'><span style='font-weight: %3;'>%1</span></div>").arg(text).arg(std::to_string(Global::fontSize)).arg(std::to_string(Global::fontWeight * 1.5)));

        leftLayout->addWidget(label1);
        rightLayout->addWidget(label2);
    }

    leftLayout->setAlignment(Qt::AlignRight);
    rightLayout->setAlignment(Qt::AlignLeft);

    mainColumnLayout->addLayout(leftLayout, 1);
    mainColumnLayout->addWidget(hspacer());
    mainColumnLayout->addLayout(rightLayout, 1);
    mainLayout->addLayout(mainColumnLayout);

    QPushButton* moreInfoButton = new QPushButton("More Info");
    QPushButton* eyeButton = new QPushButton();
    QStringList bottomText = LocalTabPage::bottomText();
    QHBoxLayout* buttonLayout = new QHBoxLayout;

    eyeButton->setIcon(showPrivate ? *CIcon::eyeClosed()->build() : *CIcon::eye()->build());
    eyeButton->setIconSize(QSize(16, 16));
    eyeButton->setFixedSize(eyeButton->sizeHint());
    buttonLayout->addWidget(moreInfoButton);
    buttonLayout->addWidget(eyeButton);
    mainLayout->addWidget(new QWidget, 1);
    mainLayout->addLayout(buttonLayout);

    for (int i = 0; i < bottomText.length(); i++) {
        QString line = bottomText[i];
        QLabel* label = new QLabel;
        QFont font = label->font();
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(label);

        font.setPointSize(8);
        effect->setOpacity(0.75);
        label->setAlignment(Qt::AlignCenter);
        label->setText(line);
        label->setFont(font);
        mainLayout->addWidget(label);
    }

    QObject::connect(moreInfoButton, &QPushButton::clicked, [=]() {
        Logger::print("moreInfoButton pressed");
        QStringList args = QCoreApplication::arguments();
        args.removeFirst();
        args.removeAll("--classic");
        QString program = QCoreApplication::applicationFilePath();
        QProcess::startDetached(program, args);
        QCoreApplication::quit();
    });

    QObject::connect(ThemeListener::instance(), &ThemeListener::themeChanged, parent, [=]() {
        eyeButton->setIcon(showPrivate ? *CIcon::eyeClosed()->build() : *CIcon::eye()->build());
    });

    QObject::connect(eyeButton, &QPushButton::clicked, parent, [=]() mutable {
        Logger::print("eyeButton pressed");
        showPrivate = !showPrivate;

        if (serialValueLabel != nullptr) {
            QString text = showPrivate ? QString::fromStdString(results["Serial"].get<std::string>()) : "";
            serialValueLabel->setText(QString("<div style='font-size: %2pt;'><span style='font-weight: %3;'>%1</span></div>").arg(text).arg(std::to_string(Global::fontSize)).arg(std::to_string(Global::fontWeight * 1.5)));
        }

        eyeButton->setIcon(showPrivate ? *CIcon::eyeClosed()->build() : *CIcon::eye()->build());
        central->update();
    });

    mainLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    central->setLayout(mainLayout);
    parent->setCentralWidget(central);
    return central;
}
