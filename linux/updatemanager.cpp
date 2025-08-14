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
    message->setAttribute(Qt::WA_DeleteOnClose);
    message->show();

    QObject::connect(reply, &QNetworkReply::finished, this, [this, reply, gui, implicit]() {
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
                    for (const QJsonValue& value : document.array()) {
                        if (value.isObject()) {
                            QJsonObject release = value.toObject();
                            QString releaseVersion = release["tag_name"].toString();
                            Version versionObject = Version::parse(releaseVersion);

                            if (versionObject <= currentversion) {
                                Logger::print(QString("Hit end of version hunt at: %1").arg(releaseVersion));
                                break;
                            }

                            bool beta = release["prerelease"].toBool();
                            bool isPublic = release["draft"].toBool();
                            bool isValidReleaseType = beta == false || useBeta;
                            bool containsValidRelease = false;

                            for (const QJsonValue& asset : release["assets"].toArray()) {
                                QString name = asset["name"].toString();
                                if (name.contains(QString("linux-%1").arg(isx86 ? "x64" : "arm64"))) containsValidRelease = true;
                            }

                            if (isPublic && isValidReleaseType && containsValidRelease) {
                                json result;
                                result["title"] = release["name"].toString().toStdString();
                                result["body"] = release["body"].toString().toStdString();
                                result["version"] = releaseVersion.toStdString();
                                result["published"] = release["published_at"].toString().toStdString();
                                result["url"] = release["url"].toString().toStdString();
                                result["beta"] = beta;

                                status = 1;
                                break;
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

        reply->deleteLater();

        if (status < 0 /* bad */) {
            if (gui) QMessageBox::critical(nullptr, "Error", "We were unable to check for updates.");
            return false;
        } else if (status == 0 || version == std::nullopt /* none found */) {
            if (gui && implicit) QMessageBox::information(nullptr, "Check for Updates", "No updates found.");
            return false;
        } else /* one found */ {
            if (gui) {
                json update = version;
                QString message = QString("A new update was found!\n\nTitle: %1\nVersion: %2 (%3)\nPublished: %4\nURL: %5\n\n%6\n\nDo you want to open it on GitHub?").arg(update["title"]).arg(update["version"]).arg(update["beta"] ? "beta" : "release").arg(update["published"]).arg(update["url"]).arg(update["body"]);
                QMessageBox::information(nullptr, "Check for Updates", message, QMessageBox::Yes | QMessageBox::Close);
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
