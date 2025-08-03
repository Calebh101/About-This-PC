#include "mainwindow.h"
#include "logger.h"

#include <QApplication>
#include <QTimer>
#include <QPainterPath>
#include "QErrorMessage"
#include <QFile>
#include "global.h"

bool isGnome() {
    const char* desktop = std::getenv("XDG_CURRENT_DESKTOP");
    if (desktop && std::string(desktop).find("GNOME") != std::string::npos) return true;
    desktop = std::getenv("DESKTOP_SESSION");
    if (desktop && std::string(desktop).find("gnome") != std::string::npos) return true;
    return false;
}

bool hasGnomePlugins() {
    QString path = QCoreApplication::libraryPaths().constFirst() + "/platformthemes/libqgtk3.so";
    return QFile::exists(path);
}

int main(int argc, char *argv[])
{
    bool classic = false;
    qputenv("QT_FONT_DPI", "96");
    Logger::enableLogging();
    Logger::setVerbose(false);
    Logger::print(QString("Starting application... (version: %1) (qt: %2)").arg(QString::fromStdString(Global::version), QT_VERSION_STR));

    QApplication a(argc, argv);
    QStringList args = QCoreApplication::arguments();
    if (args.contains("--classic")) classic = true;
    std::unique_ptr<MainWindow> w = std::make_unique<MainWindow>(classic);
    QSize size;

    if (classic) {
        size = QSize(350, 500);
    } else {
        size = QSize(550, 300);
    }

    w->setWindowTitle("About This PC");
    w->setFixedSize(size);
    w->show();

    Logger::print("Starting main window...");
    return a.exec();
}
