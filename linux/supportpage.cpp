#include "supportpage.h"
#include <QWidget>
#include "global.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDesktopServices>
#include "logger.h"

SupportPage::SupportPage() {}

QWidget* SupportPage::page(QWidget* parent, json urls) {
    QWidget* page = new QWidget(parent);
    QVBoxLayout* layout = new QVBoxLayout(page);
    QGridLayout* grid = new QGridLayout();

    QLabel* titleLabel = new QLabel;
    QFont titleFont;
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setText("Support");
    titleLabel->setAlignment(Qt::AlignCenter);

    int i = 0;

    for (auto& [key, value] : urls.items()) {
        int row = i / 2;
        int column = i % 2;
        QPushButton *button = new QPushButton(QString::fromStdString(key));

        QObject::connect(button, &QPushButton::clicked, [=]() {
            QUrl url = QUrl(QString::fromStdString(value.get<std::string>()));
            Logger::print(QString("Opening URL %2: %1").arg(url.toDisplayString(), key));
            QDesktopServices::openUrl(url);
        });

        grid->addWidget(button, row, column);
        i++;
    }

    layout->addWidget(titleLabel);
    layout->addLayout(grid);
    page->setLayout(layout);
    return page;
}
