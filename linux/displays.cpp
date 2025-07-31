#include "displays.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include "logger.h"

Displays::Displays() {}

std::string getDisplayServer() {
    const char* session = std::getenv("XDG_SESSION_TYPE");
    return session ? session : "";
}

QWidget* Displays::page(QWidget* parent) {
    QWidget *page = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(page);
    std::string server = getDisplayServer();

    if (server == "X11") {
        //
    } else {
        QLabel* titleLabel = new QLabel;
        QFont titleFont;
        titleFont.setPointSize(16);
        titleFont.setBold(true);
        titleLabel->setFont(titleFont);
        titleLabel->setText("Whoops!");
        titleLabel->setAlignment(Qt::AlignCenter);

        QLabel* messageLabel = new QLabel;
        messageLabel->setTextFormat(Qt::RichText);
        messageLabel->setText(QString("The <span style='font-weight: bold;'>%1</span> display server is currently not supported.").arg(server));
        messageLabel->setAlignment(Qt::AlignCenter);

        layout->addWidget(titleLabel);
        layout->addWidget(messageLabel);
        layout->setAlignment(Qt::AlignCenter);
    }

    Logger::print(QString("Generated page for display server: %1").arg(server));
    return page;
}
