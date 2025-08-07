#include "supportpage.h"
#include <QWidget>
#include "global.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QDesktopServices>
#include "logger.h"
#include <QGraphicsOpacityEffect>
#include "tabpage.h"

SupportPage::SupportPage() {}

QWidget* SupportPage::page(QWidget* parent, ordered_json urls) {
    QWidget* page = new QWidget(parent);
    QVBoxLayout* layout = new QVBoxLayout(page);
    QHBoxLayout* grid = new QHBoxLayout();
    QStringList bottomText = LocalTabPage::bottomText();
    QVBoxLayout* bottomTextLayout = new QVBoxLayout();

    QLabel* titleLabel = new QLabel(parent);
    QFont titleFont;
    QVBoxLayout* column1 = new QVBoxLayout();
    QVBoxLayout* columnCenter = new QVBoxLayout();
    QVBoxLayout* column2 = new QVBoxLayout();

    titleFont.setPointSize(16);
    titleLabel->setFont(titleFont);
    titleLabel->setText("Support");
    titleLabel->setAlignment(Qt::AlignCenter);
    columnCenter->addWidget(titleLabel);
    columnCenter->addSpacerItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

    int i = 0;

    for (auto& [key, value] : urls.items()) {
        int column = i % 2;
        QPushButton *button = new QPushButton(QString::fromStdString(key));

        QObject::connect(button, &QPushButton::clicked, [=]() {
            QUrl url = QUrl(QString::fromStdString(value.get<std::string>()));
            Logger::print(QString("Opening URL %2: %1").arg(url.toDisplayString(), key));
            QDesktopServices::openUrl(url);
        });

        if (column == 0) {
            column1->addWidget(button);
        } else if (column == 1) {
            column2->addWidget(button);
        } else {
            Logger::warn(QString("Error: support button %1 did not qualify for a column.").arg(key));
        }

        i++;
    }

    grid->addLayout(column1);
    grid->addLayout(columnCenter);
    grid->addLayout(column2);
    layout->addLayout(grid, 1);

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
        columnCenter->addWidget(label);
    }

    page->setLayout(layout);
    return page;
}
