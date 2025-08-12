#include "mainwindow.h"
#include "logger.h"
#include "iostream"
#include <QApplication>
#include <QTimer>
#include <QPainterPath>
#include "QErrorMessage"
#include <QFile>
#include "global.h"
#include <QDir>
#include <QProcess>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QSharedMemory>
#include <QLocalSocket>
#include <QLocalServer>

int main(int argc, char *argv[])
{
    QString id = "AboutThisPCLinuxApplication";
    bool classic = false;
    bool noWindow = false;
    qputenv("QT_FONT_DPI", "96"); // Fixed size, since sudo and other environments likes to be weird with text size

    QApplication a(argc, argv);
    QStringList args = QCoreApplication::arguments();
    QSharedMemory sharedMemory(id);
    a.setQuitOnLastWindowClosed(false);

#ifdef QT_DEBUG
    Logger::enableLogging();
    Logger::setVerbose(false);
#endif

    if (args.contains("--version")) {
        std::cout << Global::version << std::endl;
        return 0;
    }

    if (args.contains("--verbose"))  {
        Logger::enableLogging();
        Logger::enableVerbose();
    }

    if (args.contains("--classic")) classic = true;
    if (args.contains("--no-window")) noWindow = true;

    if (!sharedMemory.create(1)) {
        Logger::print("Process is already running.");
        QLocalSocket socket;
        socket.connectToServer(id);

        if (socket.waitForConnected(500)) {
            socket.write("show");
            socket.flush();
            socket.waitForBytesWritten(1000);
            return 0;
        } else {
            Logger::error("Process is running, but server is not found!");
            Logger::warn("The process is currently using shared memory, but a local server was not found. This may result in some broken functions.");

            if (sharedMemory.isAttached()) sharedMemory.detach();
            if (sharedMemory.attach()) sharedMemory.detach();
        }
    }

    if (Global::isElevated()) {
        Logger::warn("WARNING: Running as root will have unintended consequences!");
    }

    QLocalServer server;
    bool listening = false;

    if (!server.listen(id)) {
        QLocalServer::removeServer(id);
        if (!server.listen(id)) {
            Logger::warn("Unable to start local server. This may result in some broken functions.");
        } else {
            listening = true;
        }
    } else {
        listening = true;
    }

    // Log that the app is starting, even when logging is disabled
    Logger::print(QString("Starting application... (version: %1) (qt: %2)").arg(QString::fromStdString(Global::version), QT_VERSION_STR), true);

    MainWindow* w;
    QSystemTrayIcon *trayEntry = new QSystemTrayIcon();
    QMenu* trayMenu = new QMenu();

    // Load the helper binary from our resources
    QFile helper(":/binaries/helper");
    QString outPath = QDir::temp().filePath("AboutThisPC-linux-helper");
    Logger::print(QString("Loading helper at %1...").arg(outPath));

    if (listening) {
        QObject::connect(&server, &QLocalServer::newConnection, &server, [&]() {
            QLocalSocket *connection = server.nextPendingConnection();

            QObject::connect(connection, &QLocalSocket::readyRead, connection, [connection, w]() {
                QByteArray data = connection->readAll();
                if (data.startsWith("show")) MainWindow::openNewWindow(data.contains("--classic"));
                connection->disconnectFromServer();
            });
        });
    }

    if (helper.open(QIODevice::ReadOnly)) {
        if (QFile::exists(outPath)) QFile::remove(outPath);
        QFile outFile(outPath);

        if (outFile.open(QIODevice::WriteOnly)) {
            outFile.write(helper.readAll());
            outFile.close();
            QFile::setPermissions(outPath, QFileDevice::ExeUser | QFileDevice::ReadUser | QFileDevice::WriteUser);
        }

        helper.close();
        QString command = (noWindow || Global::isElevated() ? QString("%1") /* Don't run pkexec if the user doesn't want a GUI, or we already have sudo permissions */ : QString("pkexec %1")).arg(outPath); // Ask to run it
        Logger::print(QString("Running helper with command: %1").arg(command));

        std::string output = Global::run(command.toStdString());
        Global::setHelperData(output);
        QFile::remove(outPath); // Remove the helper once it's duties have finished
    } else {
        Logger::warn(QString("Unable to load helper data! File %1 could not be opened. Recovering...").arg(outPath));
    }

    if (!Global::checkHelperData()) {
        Logger::warn("Unable to load helper data! Helper did not output. Recovering...");
    }

    if (!noWindow /* yes window */) {
        Logger::print("Window is to be shown");
        w = new MainWindow(classic);

        w->setWindowTitle("About This PC");
        w->setFixedSize(MainWindow::getWindowSize(classic));
        w->show();
        MainWindow::addWindow(w);
    }

    trayEntry->setToolTip("About This PC");
    trayEntry->setIcon(QIcon(":/appicon/toolbar"));

    QAction open("About This PC", &a);
    QObject::connect(&open, &QAction::triggered, [&]() {MainWindow::openNewWindow(false);});
    trayMenu->addAction(&open);

    QAction openClassic("About This PC (Classic)", &a);
    QObject::connect(&openClassic, &QAction::triggered, [&]() {MainWindow::openNewWindow(true);});
    trayMenu->addAction(&openClassic);

    trayMenu->addSeparator();
    QAction closeWindow("Close", &a);
    QAction closeAll("Close All", &a);
    trayMenu->addAction(&closeWindow);
    trayMenu->addAction(&closeAll);

    trayMenu->addSeparator();
    QAction restart("Restart", &a);

    QAction quit("Quit", &a);
    QObject::connect(&quit, &QAction::triggered, &a, &QApplication::quit);
    trayMenu->addAction(&quit);

    QObject::connect(&restart, &QAction::triggered, [&]() {
        QString program = QCoreApplication::applicationFilePath();
        QStringList arguments = QCoreApplication::arguments();
        QProcess::startDetached(program, arguments);
        QCoreApplication::quit();
    });

    QObject::connect(&closeWindow, &QAction::triggered, &a, [&]() {
        MainWindow::removeMostRecentWindow();
    });

    QObject::connect(&closeAll, &QAction::triggered, &a, [&]() {
        MainWindow::closeAllWindows();
    });

    trayMenu->addAction(&restart);
    Logger::print("Showing system tray menu...");
    trayEntry->setContextMenu(trayMenu);
    trayEntry->show();

    Logger::print("Starting main application...");
    return a.exec();
}
