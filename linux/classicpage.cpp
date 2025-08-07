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

    json osInfo = Global::getOS();
    json chassis = Global::getChassis();
    json details = LocalTabPage::getDetails(parent);
    ordered_json results = details["results"];
    std::string localIPName = details["localIPName"];

    std::string productFamily = Global::getFamily();
    std::string productName = Global::getModel();
    QStringList localIPString;

    mainLayout->addWidget(LocalTabPage::processImage(chassis["icon"], parent, 184));
    mainLayout->addItem(vspacer());

    QLabel* familyLabel = new QLabel(parent);
    QFont familyFont = familyLabel->font();
    familyLabel->setText(QString::fromStdString(productFamily));
    familyFont.setPointSize(24);
    familyLabel->setFont(familyFont);
    familyLabel->setAlignment(Qt::AlignHCenter);
    mainLayout->addWidget(familyLabel);

    QLabel* productLabel = new QLabel(parent);
    productLabel->setText(QString::fromStdString(productName));
    QFont productFont = productLabel->font();
    QLabel* serialValueLabel = nullptr;
    QLabel* localIPLabel = nullptr;

    productFont.setPointSize(Global::fontSize);
    productLabel->setFont(productFont);
    productLabel->setAlignment(Qt::AlignHCenter);
    mainLayout->addWidget(productLabel);
    mainLayout->addItem(vspacer());

    for (auto& [key, value] : results.items()) {
        QLabel* label1 = new QLabel(parent);
        QLabel* label2 = new QLabel(parent);
        QString text = QString::fromStdString(value.dump());

        if (value.is_string()) text = QString::fromStdString(value.get<std::string>());
        if (key == "Serial") serialValueLabel = label2;
        if (key == localIPName) localIPLabel = label2;

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
        QLabel* label = new QLabel(parent);
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

        if (localIPLabel != nullptr) {
            QString text = showPrivate ? QString::fromStdString(results[localIPName].get<std::string>()) : "";
            localIPLabel->setText(QString("<div style='font-size: %2pt;'><span style='font-weight: %3;'>%1</span></div>").arg(text).arg(std::to_string(Global::fontSize)).arg(std::to_string(Global::fontWeight * 1.5)));
        }

        eyeButton->setIcon(showPrivate ? *CIcon::eyeClosed()->build() : *CIcon::eye()->build());
        central->update();
    });

    mainLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    central->setLayout(mainLayout);
    parent->setCentralWidget(central);
    return central;
}
