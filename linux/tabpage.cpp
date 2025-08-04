#include "tabpage.h"
#include <qapplication.h>
#include <qboxlayout.h>
#include <qlabel.h>
#include "json.hpp"
#include <string>
#include <vector>
#include "logger.h"
#include <optional>
#include <QPainter>
#include <QPainterPath>
#include "global.h"
#include "cicon.h"
#include "themelistener.h"
#include <QPushButton>
#include <QGraphicsOpacityEffect>
#include <QProcess>
#include <QInputDialog>
#include <QLineEdit>

namespace fs = std::filesystem;
using json = nlohmann::json;

std::optional<fs::path> _getIconPath(std::string id) {
    std::vector<std::string> icon_dirs = {
        "/usr/share/icons/hicolor/48x48/apps/",
        "/usr/share/icons/hicolor/scalable/apps/",
        "/usr/share/pixmaps/"
    };

    std::vector<std::string> extensions = {".png", ".svg"};

    for (const auto& dir : icon_dirs) {
        for (const auto& ext : extensions) {
            fs::path icon_path = fs::path(dir) / (id + ext);
            if (fs::exists(icon_path)) return icon_path;
        }
    }

    return std::nullopt;
}

std::optional<fs::path> LocalTabPage::getIconPath(std::string id) {
    std::optional<fs::path> result = _getIconPath(id);
    Logger::print(QString("Found icon file: %1").arg((!result ? std::string("none") : result->string())));
    return result;
}

QStringList LocalTabPage::bottomText() {
    QStringList results;
    results.append(QString("About This PC %1 by Calebh101").arg(Global::version));
    return results;
}

QWidget* LocalTabPage::processImage(std::optional<fs::path> path, QWidget* parent, int size, int radius) {
    QWidget* container = new QWidget();
    QString iconPath = path ? QString::fromStdString(path->string()) : ":default-linux-icon/images/default-linux-icon.png";
    Logger::print(QString("Processing image of path %1").arg(iconPath));
    QIcon icon(iconPath);
    QLabel* label = new QLabel(parent);
    QPixmap pixmap = icon.pixmap(size, size);

    QPixmap rounded(pixmap.size());
    rounded.fill(Qt::transparent);
    QPainter painter(&rounded);
    painter.setRenderHint(QPainter::Antialiasing);
    QPainterPath paintPath;
    paintPath.addRoundedRect(pixmap.rect(), radius, radius);
    painter.setClipPath(paintPath);
    painter.drawPixmap(0, 0, pixmap);

    label->setPixmap(rounded);
    QVBoxLayout* layout = new QVBoxLayout(container);
    layout->addWidget(label, 0, Qt::AlignCenter);
    container->setLayout(layout);
    return container;
}

LocalTabPage::LocalTabPage() {}

QWidget* LocalTabPage::overview(QWidget* parent) {
    bool showPrivate = true;
    bool showLockButton = false;
    bool isElevated = Global::isElevated();
    float fontSize = Global::fontSize;
    int fontWeight = Global::fontWeight;

    QWidget *page = new QWidget();
    QVBoxLayout *vlayout = new QVBoxLayout();
    QVBoxLayout *infoWidget = new QVBoxLayout();
    QHBoxLayout *layout = new QHBoxLayout();
    QHBoxLayout* bottomLayout = new QHBoxLayout();
    QVBoxLayout* bottomTextLayout = new QVBoxLayout();
    ordered_json results;

    json chassis = Global::getChassis();
    json cpuInfo = Global::getCPU();
    json gpuInfo = Global::getGPU();
    json osInfo = Global::getOS();
    json ramInfo = Global::getHelperData("memory");
    json serialInfo = Global::getHelperData("serial");

    std::optional<std::string> iconId = Global::atKeyOrNull<std::string>(osInfo, "LOGO");
    std::optional<fs::path> iconPath = !iconId ? std::nullopt : getIconPath(*iconId);
    Logger::print(QString("Found icon path: %1").arg(iconPath ? iconPath->string() : "none"));
    std::vector<json> localIPs = Global::getLocalIPs();
    QStringList localIPString;
    QString localIPName;

    for (int i = 0; i < localIPs.size(); i++) {
        json item = localIPs[i];
        QString text = QString("%1:%2").arg(item["interface"].get<std::string>()).arg(item["ip"].get<std::string>());
        localIPString.push_back(text);
    }

    QLabel* title = new QLabel(parent);
    title->setText(QString::fromStdString(osInfo["PRETTY_NAME"].get<std::string>()));
    QFont font = title->font();
    float speed = cpuInfo["speed"].get<float>();
    std::ostringstream oss;
    oss << std::defaultfloat << std::setprecision(2) << speed;
    std::string speedString = oss.str();
    std::string processorString = QString::fromStdString(cpuInfo["processors"].front().get<std::string>()).toStdString();
    results["Processor"] = QString("%3 %1GHz %2").arg(speedString).arg(processorString).arg(cpuInfo["arch"].get<std::string>()).toStdString();

    std::vector<std::string> ramAttributes;
    std::string productFamily = Global::getFamily();
    std::string productName = Global::getModel();
    std::string startupDiskPath = Global::getStartupDisk();
    json startupDiskInfo = Global::getDisk(startupDiskPath);
    QLabel* serialValueLabel = nullptr;
    QLabel* localIPLabel = nullptr;
    QStringList subtitleItems;

    if (ramInfo.contains("totalString")) ramAttributes.push_back(ramInfo["totalString"]);
    if (ramInfo.contains("type")) ramAttributes.push_back(ramInfo["type"]);
    if (ramInfo.contains("form")) ramAttributes.push_back(ramInfo["form"]);
    if (ramInfo.contains("speed")) ramAttributes.push_back(ramInfo["speed"]);

    if (gpuInfo["status"] == true) {
        json gpu = gpuInfo["gpus"].front();
        int vram = gpu["vram"].get<int>();
        std::ostringstream oss;
        oss << std::defaultfloat << std::setprecision(2) << (vram / 1000.0);
        QString text = QString("%1 %2GB").arg(gpu["name"].get<std::string>()).arg(oss.str());
        results["Graphics"] = text.toStdString();
    }

    if (!ramAttributes.empty()) {
        std::ostringstream oss;

        for (size_t i = 0; i < ramAttributes.size(); ++i) {
            if (i > 0) oss << ' ';
            oss << ramAttributes[i];
        }

        results["Memory"] = oss.str();
    }

    if (serialInfo.contains("serial")) {
        results["Serial"] = serialInfo["serial"];
    }

    if (startupDiskInfo["status"] == true) {
        long long bytes = startupDiskInfo["bytes"].get<long long>();
        int size = bytes / 1000.0 / 1000.0 / 1000.0;
        results["Startup Disk"] = QString("%1 %2 GB").arg(startupDiskPath).arg(std::to_string(size)).toStdString();
    } else {
        results["Startup Disk"] = startupDiskPath;
    }

    if (!localIPString.empty()) {
        bool multiple = localIPString.size() > 1;
        localIPName = QString("Local %1").arg(multiple ? "IPs" : "IP");
        results[localIPName.toStdString()] = localIPString.join(", ").toStdString();
    }

    results["Kernel"] = osInfo["kernel"];
    QLabel* modelLabel = new QLabel(parent);
    modelLabel->setTextFormat(Qt::RichText);
    modelLabel->setText(QString("<span style='font-weight: %3; font-size: %2pt;'>%1</span>").arg(Global::getModel()).arg(std::to_string(fontSize)).arg(std::to_string(fontWeight * 1.5)));

    font.setPointSize(fontSize * 2);
    font.setWeight(static_cast<QFont::Weight>(fontWeight * 1.5));
    title->setFont(font);

    std::string releaseType = osInfo.contains("RELEASE_TYPE") ? osInfo["RELEASE_TYPE"] : "stable";
    std::string releaseTypeString = releaseType;
    Logger::print(QString("Found release type of %1").arg(releaseType));

    if (releaseType == "stable") releaseTypeString = "Stable";
    if (releaseType == "lts") releaseTypeString = "LTS";
    if (releaseType == "experiment") releaseTypeString = "Experimental";
    if (releaseType == "development") releaseTypeString = "Development";

    if (osInfo.contains("VERSION_ID")) subtitleItems.push_back(QString::fromStdString(osInfo["VERSION_ID"]));
    if (osInfo.contains("VERSION_CODENAME")) subtitleItems.push_back(QString::fromStdString(Global::toSentenceCase(osInfo["VERSION_CODENAME"])));
    if (osInfo.contains("BUILD_ID")) subtitleItems.push_back(QString("(%1)").arg(osInfo["BUILD_ID"]));
    if (releaseType != "stable") subtitleItems.push_back(QString::fromStdString(releaseTypeString)); // Sometimes OSes just don't include this even if it is LTS or Experimental, so we shouldn't force a default; so instead we omit the property.

    infoWidget->setAlignment(Qt::AlignTop);
    infoWidget->addWidget(title);

    if (!subtitleItems.empty()) {
        QLabel* label = new QLabel(parent);

        font.setPointSize(fontSize * 1.25);

        label->setText(subtitleItems.join(" "));
        label->setFont(font);
        infoWidget->addWidget(label);
    }

    infoWidget->addSpacing(10);
    infoWidget->addWidget(modelLabel);

    for (auto& [key, value] : results.items()) {
        QHBoxLayout* layout = new QHBoxLayout;
        QLabel* label1 = new QLabel(parent);
        QLabel* label2 = new QLabel(parent);
        QString text = QString::fromStdString(value.dump());

        if (value.is_string()) text = QString::fromStdString(value.get<std::string>());
        if (key == "Serial") serialValueLabel = label2;
        if (key == localIPName.toStdString()) localIPLabel = label2;

        label1->setTextFormat(Qt::RichText);
        label2->setTextFormat(Qt::RichText);
        label1->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        label2->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        label1->setText(QString("<div style='font-size: %2pt;'><span style='font-weight: %3s;'>%1</span></div>").arg(QString::fromStdString(key)).arg(std::to_string(Global::fontSize)).arg(std::to_string(Global::fontWeight)));
        label2->setText(QString("<div style='font-size: %2pt;'><span style='font-weight: %3;'>%1</span></div>").arg(text).arg(std::to_string(Global::fontSize)).arg(std::to_string(Global::fontWeight * 1.5)));

        layout->addWidget(label1);
        layout->addWidget(label2);
        layout->addStretch();
        infoWidget->addLayout(layout);
    }

    QPushButton* eyeButton = new QPushButton();
    QPushButton* lockButton = new QPushButton();
    QStringList bottomText = LocalTabPage::bottomText();

    eyeButton->setIcon(showPrivate ? *CIcon::eyeClosed()->build() : *CIcon::eye()->build());
    eyeButton->setIconSize(QSize(16, 16));
    eyeButton->setFixedSize(eyeButton->sizeHint());

    lockButton->setIcon(isElevated ? *CIcon::lockOpen()->build() : *CIcon::lock()->build());
    lockButton->setIconSize(QSize(16, 16));
    lockButton->setFixedSize(lockButton->sizeHint());
    if (isElevated) lockButton->setEnabled(false);

    QObject::connect(ThemeListener::instance(), &ThemeListener::themeChanged, parent, [=]() {
        eyeButton->setIcon(showPrivate ? *CIcon::eyeClosed()->build() : *CIcon::eye()->build());
        lockButton->setIcon(isElevated ? *CIcon::lockOpen()->build() : *CIcon::lock()->build());
    });

    QObject::connect(eyeButton, &QPushButton::clicked, parent, [=]() mutable {
        Logger::print("eyeButton pressed");
        showPrivate = !showPrivate;

        if (serialValueLabel != nullptr) {
            QString text = showPrivate ? QString::fromStdString(results["Serial"].get<std::string>()) : "";
            serialValueLabel->setText(QString("<div style='font-size: %2pt;'><span style='font-weight: %3;'>%1</span></div>").arg(text).arg(std::to_string(Global::fontSize)).arg(std::to_string(Global::fontWeight * 1.5)));
        }

        if (localIPLabel != nullptr) {
            QString text = showPrivate ? QString::fromStdString(results[localIPName.toStdString()].get<std::string>()) : "";
            localIPLabel->setText(QString("<div style='font-size: %2pt;'><span style='font-weight: %3;'>%1</span></div>").arg(text).arg(std::to_string(Global::fontSize)).arg(std::to_string(Global::fontWeight * 1.5)));
        }

        eyeButton->setIcon(showPrivate ? *CIcon::eyeClosed()->build() : *CIcon::eye()->build());
        page->update();
    });

    QObject::connect(lockButton, &QPushButton::clicked, parent, [=]() mutable {
        Logger::print("lockButton pressed");
        if (isElevated) return;

        // Couldn't get this to work properly, so I had to make a helper script instead
    });

    layout->addWidget(processImage(iconPath, parent, 148, 10), 5);
    layout->addLayout(infoWidget, 9);
    vlayout->addLayout(layout, 1);

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
        bottomTextLayout->addWidget(label);
    }

    bottomLayout->addSpacing(eyeButton->width());
    if (showLockButton) bottomLayout->addSpacing(lockButton->width());
    bottomLayout->addLayout(bottomTextLayout);
    bottomLayout->addWidget(eyeButton);
    if (showLockButton) bottomLayout->addWidget(lockButton);
    vlayout->addLayout(bottomLayout);
    page->setLayout(vlayout);
    return page;
}
