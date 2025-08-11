#include <QApplication>
#include "iostream"
#include <QDebug>
#include <QSharedMemory>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>

QString version = "0.0.0A";

void debug(QString input) {
    qDebug() << input.toStdString();
}

void raw(QString input) {
    std::cout << input.toStdString() << std::endl;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QStringList args = QCoreApplication::arguments();
    QSharedMemory sharedMemory("AboutThisPCLinuxService");

    if (args.contains("--version")) {
        raw(version);
        return 0;
    }

    if (!sharedMemory.create(1)) {
        raw("Service is already running.");
        return 0;
    }

    try {
        QSystemTrayIcon trayEntry;
        trayEntry.setToolTip("About This PC");
        trayEntry.setIcon(QIcon(":/icons/main"));

        QMenu menu;
        QAction quit("Quit Service", &a);
        QObject::connect(&quit, &QAction::triggered, &a, &QApplication::quit);
        menu.addAction(&quit);

        trayEntry.setContextMenu(&menu);
        trayEntry.show();
        return a.exec();
    } catch (...) {
        raw("Unable to show menu entry");
        return 1;
    }
}
