#include "mainwindow.h"
#include "logger.h"

#include <QApplication>
#include <QTimer>
#include <QPainterPath>
#include "QErrorMessage"
#include <QFile>
#include "global.h"
#include <QDir>
#include <QProcess>

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
    qputenv("QT_FONT_DPI", "96"); // Fixed size, since sudo likes to be weird with text size
    QApplication a(argc, argv);
    QStringList args = QCoreApplication::arguments();
    Logger::setVerbose(false);
    if (args.contains("--classic")) classic = true;

#ifdef QT_DEBUG
    Logger::enableLogging();
#endif

    if (args.contains("--verbose"))  {
        Logger::enableLogging();
        Logger::enableVerbose();
    }

    Logger::print(QString("Starting application... (version: %1) (qt: %2)").arg(QString::fromStdString(Global::version), QT_VERSION_STR));

    QFile helper(":/binaries/helper");
    QString outPath = QDir::temp().filePath("AboutThisPC-linux-helper");
    Logger::print(QString("Loading helper at %1...").arg(outPath));

    if (helper.open(QIODevice::ReadOnly)) {
        QFile outFile(outPath);

        if (outFile.open(QIODevice::WriteOnly)) {
            outFile.write(helper.readAll());
            outFile.close();
            QFile::setPermissions(outPath, QFileDevice::ExeUser | QFileDevice::ReadUser | QFileDevice::WriteUser);
        }

        helper.close();
        QString command = QString("pkexec %1").arg(outPath);
        Logger::print(QString("Running helper with command: %1").arg(command));

        std::string output = Global::run(command.toStdString());
        Global::setHelperData(output);
    } else {
        Logger::warn(QString("Unable to load helper data! File %1 could not be opened. Recovering...").arg(outPath));
    }

    std::unique_ptr<MainWindow> w = std::make_unique<MainWindow>(classic);
    QSize size;

    if (classic) {
        size = QSize(350, 500);
    } else {
        size = QSize(600, 350);
    }

    if (!Global::checkHelperData()) {
        Logger::warn("Unable to load helper data! Recovering...");
    }

    w->setWindowTitle("About This PC");
    w->setFixedSize(size);
    w->show();

    Logger::print("Starting main window...");
    return a.exec();
}
