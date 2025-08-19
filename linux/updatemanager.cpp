#include "updatemanager.h"
#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include "logger.h"
#include <QJsonArray>
#include "json.hpp"
#include <QMessageBox>
#include <QApplication>
#include "global.h"
#include <QTimer>
#include <QDesktopServices>

using json = nlohmann::json;

UpdateManager::UpdateManager() {
    manager = new QNetworkAccessManager(this);
}

void UpdateManager::check(bool gui, bool implicit) {
    QUrl url("https://api.github.com/repos/Calebh101/About-This-PC/releases");
    QNetworkRequest request(url);
    QNetworkReply* reply = manager->get(request);
    Logger::print(QString("Sending update request to %1...").arg(url.toDisplayString()));
    QMessageBox* message = new QMessageBox(QMessageBox::Information, "Loading...", "Checking for updates...");

    if (implicit) {
        message->setAttribute(Qt::WA_DeleteOnClose);
        message->show();
    }

    QObject::connect(reply, &QNetworkReply::finished, this, [this, reply, gui, implicit, url, message]() {
        Logger::print("Reply received");
        std::optional<json> version = std::nullopt;
        Version currentversion = Version::parse(QString::fromStdString(Global::version));

        int status = -1;
        bool useBeta = Global::settings().get<bool>({"isBeta"});
        bool isx86 = sizeof(void*) == 8 || sizeof(void*) == 4;

        try {
            if (reply->error() == QNetworkReply::NoError) {
                QByteArray response = reply->readAll();
                QJsonDocument document = QJsonDocument::fromJson(response);
                Logger::print(QString("Received response of %1 bytes").arg(response.length()));

                if (document.isArray()) {
                    int count = document.array().count();
                    Logger::print(QString("Input is valid and array: %1 items").arg(count));

                    for (int i = 0; i < count; i++) {
                        const QJsonValue& value = document.array()[i];

                        if (value.isObject()) {
                            QJsonObject release = value.toObject();
                            QString releaseVersion = release["tag_name"].toString();
                            Version versionObject = Version::parse(releaseVersion);
                            QJsonArray assets = release["assets"].toArray();
                                Logger::verbose(QString("Scanning release %1...").arg(releaseVersion));

                            if (versionObject <= currentversion) {
                                Logger::print(QString("Hit end of version hunt at: %1").arg(releaseVersion));
                                break;
                            }

                            bool beta = release["prerelease"].toBool();
                            bool isPublic = release["draft"].toBool() == false;
                            bool isValidReleaseType = beta == false || useBeta == true;
                            bool containsValidRelease = false;

                            for (int i = 0; i < assets.count(); i++) {
                                const QJsonValue& asset = assets[i];
                                QString name = asset["name"].toString();
                                Logger::verbose(QString("Scanning asset %1...").arg(name));
                                if (name.contains(QString("linux-%1").arg(isx86 ? "x64" : "arm64"))) containsValidRelease = true;
                            }

                            if (isPublic && isValidReleaseType && containsValidRelease) {
                                Logger::print(QString("Version hunt was successful: Found version %1").arg(releaseVersion));
                                json result;

                                result["title"] = release["name"].toString().toStdString();
                                result["body"] = release["body"].toString().toStdString();
                                result["version"] = releaseVersion.toStdString();
                                result["published"] = release["published_at"].toString().toStdString();
                                result["url"] = release["html_url"].toString().toStdString();
                                result["beta"] = beta;

                                status = 1;
                                version = result;
                                break;
                            } else {
                                Logger::verbose(QString("Version hunt will keep going: Failed version %1: %2%3%4").arg(releaseVersion).arg(isPublic).arg(isValidReleaseType).arg(containsValidRelease));
                            }
                        }
                    }

                    if (status < 0) {
                        status = 0;
                    }
                } else {
                    Logger::warn(QString("Update error: Document is not an object: %1").arg(response));
                }
            } else {
                Logger::warn(QString("Update error: %1").arg(reply->errorString()));
            }
        } catch (std::exception e) {
            Logger::warn(QString("Update error: %1").arg(e.what()));
        } catch (...) {
            Logger::warn(QString("Update error: %1").arg("Unknown error"));
        }

        if (message->isVisible()) message->close();
        reply->deleteLater();

        if (status < 0 /* bad */) {
            Logger::warn(QString("Updater: Bad response! (%1)").arg(status));
            if (gui) QMessageBox::critical(nullptr, "Error", QString("We were unable to check for updates. Make sure you're connected to the Internet and that the following URL is accessible.\n\nURL: %1").arg(url.toDisplayString()));
            return false;
        } else if (status == 0 || version == std::nullopt /* none found */) {
            Logger::print("Updater: No updates found");
            if (gui && implicit) QMessageBox::information(nullptr, "Check for Updates", "No updates found.");
            return false;
        } else /* one found */ {
            json update = version;
            Logger::print(QString("Updater: Update found: %1").arg(QString::fromStdString(update["version"].get<std::string>())));

            if (gui) {
                QUrl url = QUrl(QString::fromStdString(update["url"].get<std::string>()));
                QString message = QString("A new update was found!\n\nTitle: %1\nVersion: %2 (%3)\nPublished: %4\nURL: %5\n\n%6").arg(QString::fromStdString(update["title"].get<std::string>())).arg(QString::fromStdString(update["version"].get<std::string>())).arg(update["beta"].get<bool>() ? "beta" : "release").arg(QString::fromStdString(update["published"].get<std::string>())).arg(url.toDisplayString()).arg(QString::fromStdString(update["body"].get<std::string>()));
                QMessageBox::StandardButton reply = QMessageBox::information(nullptr, "Check for Updates", message, QMessageBox::Open | QMessageBox::Ok);

                if (reply == QMessageBox::Open) {
                    Logger::print(QString("Opening URL %1...").arg(url.toDisplayString()));
                    QDesktopServices::openUrl(url);
                }
            }

            return true;
        }
    });
}

Version::Version(int major, int minor, int subminor, int patch, int release) : aa(major), ab(minor), ac(subminor), ba(patch), ca(release) {}

Version Version::parse(QString input) {
    int major = 0;
    int minor = 0;
    int subminor = 0;
    int patch = 0;
    int release = 0;

    QStringList parts = input.split("-");
    QStringList sections = parts.first().split(".");

    if (parts.count() >= 2) {
        release = std::stoi(parts[1].replace("R", "").toStdString());
    }

    if (sections.count() >= 3) {
        QString numbers;
        QString letters;
        QString text = sections[2];

        for (QChar c : text) {
            if (c.isDigit()) {
                numbers.append(c);
            } else {
                letters.append(c);
            }
        }

        if (letters.size() != 1) {
            Logger::warn(QString("Unable to parse version string %1: Invalid letters: %2 (failed size check)").arg(input).arg(letters));
        } else {
            char letter = std::toupper(letters.toStdString()[0]);

            if (letter >= 'A' && letter <= 'Z') {
                int position = letter - 'A';
                patch = position;
            } else {
                Logger::warn(QString("Unable to parse version string %1: Invalid letters: %2 (failed range check)").arg(input).arg(letters));
            }
        }

        subminor = std::stoi(numbers.toStdString());
    }

    if (sections.count() >= 2) minor = std::stoi(sections[1].toStdString());
    if (sections.count() >= 1) major = std::stoi(sections[0].toStdString());
    return Version(major, minor, subminor, patch, release);
}
