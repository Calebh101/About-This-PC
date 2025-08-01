#include "mainwindow.h"
#include "logger.h"

#include <QApplication>
#include <QTimer>
#include <QPainterPath>
#include "QErrorMessage"
#include <QFile>

const std::string version = "0.0.0A";

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
    Logger::setVerbose(false);
    bool gnome = isGnome();
    Logger::print(QString("Starting application... (version: %1) (qt: %2)").arg(QString::fromStdString(version)).arg(QT_VERSION_STR));
    Logger::print(QString("Is GNOME: %1").arg(gnome));

    if (gnome) {
        if (!hasGnomePlugins()) {
            qFatal("libqgtk3 was not found!");
        } else {
            qputenv("QT_QPA_PLATFORMTHEME", "gtk3");
            Logger::print(QString("Enabling QT_QPA_PLATFORMTHEME of value %1").arg(std::getenv("QT_QPA_PLATFORMTHEME")));
        }
    }

    QApplication a(argc, argv);
    bool classic = true;
    QStringList args = QCoreApplication::arguments();
    if (args.contains("--classic")) classic = true;
    MainWindow w = new MainWindow(classic);

    if (classic) {
        w.resize(350, 400);
    } else {
        w.resize(550, 300);
    }

    w.setWindowTitle("About This PC");
    w.setMinimumSize(w.size());
    w.setMaximumSize(w.size());
    w.show();

    Logger::print("Starting main window...");
    return a.exec();
}
