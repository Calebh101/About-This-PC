#include "settings.hpp"
#include "json.hpp"
#include <QString>
#include "logger.h"
#include <QWidget>
#include <QTreeView>
#include <QStandardItemModel>
#include "global.h"
#include <QComboBox>
#include <QVBoxLayout>
#include <QLabel>
#include <qpushbutton.h>
#include <qscrollarea.h>
#include "mainwindow.h"

namespace fs = std::filesystem;
using json = nlohmann::json;

QString Settings::file = QString("%1/.AboutThisPC/settings.json").arg(std::getenv("HOME"));

Settings::Settings() {
    this->reload();
}

QWidget* title(QWidget* parent, QString title, QString description) {
    QWidget* container = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(container);

    QLabel* titleLabel = new QLabel(parent);
    QFont font = titleLabel->font();

    QLabel* desc = new QLabel(parent);
    QFont descFont = desc->font();

    font.setPointSize(20);
    descFont.setPointSize(9);

    titleLabel->setText(title);
    titleLabel->setFont(font);

    desc->setText(description);
    desc->setFont(descFont);

    layout->addWidget(titleLabel);
    layout->addWidget(desc);
    return container;
}

QWidget* setting(QWidget* parent, QString title, QString description, QWidget* selector) {
    QWidget* container = new QWidget();
    QHBoxLayout* mainLayout = new QHBoxLayout(container);
    QVBoxLayout* textLayout = new QVBoxLayout();

    QLabel* titleLabel = new QLabel(container);
    QLabel* descLabel = new QLabel(container);

    QFont titleFont = titleLabel->font();
    QFont descFont = descLabel->font();

    titleFont.setPointSize(12);
    descFont.setPointSize(9);

    titleLabel->setFont(titleFont);
    descLabel->setFont(descFont);
    titleLabel->setText(title);
    descLabel->setText(description);
    titleLabel->setWordWrap(true);
    descLabel->setWordWrap(true);

    textLayout->addWidget(titleLabel);
    textLayout->addWidget(descLabel);
    mainLayout->addLayout(textLayout, 1);
    mainLayout->addWidget(selector);

    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    return container;
}

QWidget* Selector::boolean(QWidget* parent, QStringList keys) {
    bool value = Global::settings().get<bool>(keys);
    QComboBox* box = new QComboBox(parent);
    box->addItems({"Yes", "No"});
    box->setCurrentText(value ? "Yes" : "No");

    QObject::connect(box, &QComboBox::currentTextChanged, parent, [=](const QString &text){
        Logger::print(QString("Setting changed to: %1").arg(text));
        Global::settings().set<bool>(text == "Yes", keys);
    });

    return box;
}

QWidget* Selector::button(QWidget* parent, QString text, QString tooltip, std::function<void()> callback) {
    QPushButton* button = new QPushButton(text, parent);
    button->setToolTip(tooltip);
    QObject::connect(button, &QPushButton::clicked, callback);
    return button;
}

QWidget* Settings::page(QWidget* window) {
    QWidget* container = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(container);
    QScrollArea* scroll = new QScrollArea();

    layout->addWidget(title(container, "General", "General settings for About This PC."));
    layout->addWidget(setting(container, "Use Beta Versions", "Allow the Update Checker to allow beta (prerelease) versions.", Selector::boolean(container, {"isBeta"})));
    layout->addWidget(setting(container, "Check for Updates at Start", "When the About This PC service starts, check for updates automatically.", Selector::boolean(container, {"checkForUpdatesAtStart"})));

    layout->addWidget(setting(container, "Reset All Settings", "Reset all About This PC settings to default.", Selector::button(container, "Reset", "Reset all About This PC settings to default.", [window] {
        Logger::print("Reset called");
        Global::settings().reset();
        window->close();
    })));

    scroll->setWidgetResizable(true);
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    container->setFixedHeight(container->sizeHint().height());
    MainWindow::processParent(container);
    scroll->setWidget(container);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    return scroll;
}

void Settings::window(QWidget* parent) {
    QWidget* w = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout();

    layout->addWidget(page(w), 1);
    w->setWindowTitle("About This PC Settings");
    w->resize(MainWindow::getWindowSize());
    w->setMinimumSize(w->size());
    w->setLayout(layout);
    w->show();
}
