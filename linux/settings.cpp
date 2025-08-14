#include "settings.hpp"
#include "json.hpp"
#include <QString>
#include <fstream>
#include <filesystem>
#include "logger.h"
#include <QWidget>
#include <QTreeView>
#include <QStandardItemModel>
#include "global.h"
#include <QComboBox>
#include <QVBoxLayout>
#include <QLabel>
#include "mainwindow.h"

namespace fs = std::filesystem;
using json = nlohmann::json;

Settings::Settings() {
    QString directory = QString("%1/.AboutThisPC").arg(std::getenv("HOME"));
    QString settingsfile = QString("%1/%2").arg(directory, "settings.json");

    if (!fs::exists(directory.toStdString()) || !fs::is_directory(directory.toStdString())) {
        Logger::print(QString("Creating directory at %1...").arg(directory), true);
        fs::create_directory(directory.toStdString());
    }

    std::ifstream infile(settingsfile.toStdString());
    std::stringstream buffer;
    bool status = false;
    buffer << infile.rdbuf();

    if (infile.good()) {
        try {
            this->loaded = json::parse(buffer.str());
            status = true;
        } catch (...) {
            Logger::warn(QString("Unable to parse settings file! Recovering..."));
        }
    }

    if (status == false) {
        Logger::print("Loading default settings...", true);
        json j = defaults();
        this->loaded = j;

        std::ofstream outfile(settingsfile.toStdString());

        if (outfile.is_open()) {
            outfile << j.dump() << std::endl;
        } else {
            Logger::print(QString("Unable to write to settings file %1: File not open").arg(settingsfile));
        }
    }

    Logger::print("Continuing...");
}

QWidget* title(QWidget* parent, QString text) {
    QLabel* result = new QLabel(parent);
    QFont font = result->font();

    font.setPointSize(20);
    result->setText(text);
    result->setFont(font);

    return result;
}

QWidget* setting(QWidget* parent, QString title, QString description, QWidget* selector) {
    QWidget* container = new QWidget();
    QHBoxLayout* mainLayout = new QHBoxLayout(container);
    QVBoxLayout* textLayout = new QVBoxLayout();

    QLabel* titleLabel = new QLabel(container);
    QLabel* descLabel = new QLabel(container);

    QFont titleFont = titleLabel->font();
    QFont descFont = descLabel->font();

    titleFont.setPointSize(16);
    descFont.setPointSize(10);

    titleLabel->setFont(titleFont);
    descLabel->setFont(descFont);
    titleLabel->setText(title);
    descLabel->setText(description);

    textLayout->addWidget(titleLabel);
    textLayout->addWidget(descLabel);
    mainLayout->addLayout(textLayout);
    mainLayout->addWidget(selector);
    return container;
}

QWidget* booleanSelector(QWidget* parent, QStringList keys) {
    bool value = Global::settings().get<bool>(keys);
    QComboBox* box = new QComboBox(parent);
    box->setCurrentText(value ? "Yes" : "No");
    box->addItems({"Yes", "No"});

    QObject::connect(box, &QComboBox::currentTextChanged, parent, [=](const QString &text){
        Logger::print(QString("Setting changed to: %1").arg(text));
        Global::settings().set<bool>(text == "Yes", keys);
    });

    return box;
}

QWidget* Settings::page(QWidget* parent) {
    QWidget* container = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(container);

    layout->addWidget(title(container, "General"));
    layout->addWidget(setting(container, "Use Beta Versions", "Allow the Update Checker to allow beta (prerelease) versions.", booleanSelector(container, {"isBeta"})));

    return container;
}

void Settings::window(QWidget* parent) {
    QWidget* w = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout();

    layout->addWidget(page(w));
    w->setWindowTitle("About This PC Settings");
    w->resize(MainWindow::getWindowSize());
    w->setLayout(layout);
    w->show();
}
