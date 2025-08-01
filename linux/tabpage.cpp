#include "tabpage.h"
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

std::optional<fs::path> getIconPath(std::string& id) {
    std::optional<fs::path> result = _getIconPath(id);
    Logger::print(QString("Found icon file: %1").arg((!result ? std::string("none") : result->string())));
    return result;
}

QWidget* processImage(std::optional<fs::path> path, int radius, int size) {
    QWidget* container = new QWidget();
    QString iconPath = path ? QString::fromStdString(path->string()) : ":/images/default-linux-icon.png";
    QIcon icon(iconPath);
    QLabel* label = new QLabel;
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
    float fontSize = Global::fontSize;
    int fontWeight = Global::fontWeight;

    QWidget *page = new QWidget();
    QVBoxLayout *infoWidget = new QVBoxLayout();
    QHBoxLayout *layout = new QHBoxLayout(page);

    json results;
    json osInfo = Global::getOS();
    json cpuInfo = Global::getCPU();

    std::optional<std::string> iconId = Global::atKeyOrNull<std::string>(osInfo, "LOGO");
    std::optional<fs::path> iconPath = !iconId ? std::nullopt : getIconPath(*iconId);
    Logger::print(QString("Found icon path: %1").arg(iconPath ? iconPath->string() : "none"));

    QLabel* title = new QLabel(QString::fromStdString(osInfo["PRETTY_NAME"].get<std::string>()));
    QString cpuModel = QString::fromStdString(cpuInfo["model name"].get<std::string>());
    results["Processor"] = cpuModel.toStdString();
    QFont font = title->font();

    QLabel* modelLabel = new QLabel;
    modelLabel->setTextFormat(Qt::RichText);
    modelLabel->setText(QString("<span style='font-weight: %3; font-size: %2pt;'>%1</span>").arg(Global::getModel()).arg(std::to_string(fontSize)).arg(std::to_string(fontWeight * 1.5)));

    font.setPointSize(fontSize * 2);
    font.setWeight(static_cast<QFont::Weight>(fontWeight * 1.5));
    title->setFont(font);

    infoWidget->setAlignment(Qt::AlignTop);
    infoWidget->addWidget(title);
    infoWidget->addWidget(modelLabel);

    for (auto& [key, value] : results.items()) {
        QLabel* label = new QLabel;
        QString text = QString::fromStdString(value.dump());
        if (value.is_string()) text = QString::fromStdString(value.get<std::string>());
        label->setTextFormat(Qt::RichText);
        label->setText(QString("<div style='font-size: %3pt;'><span style='font-weight: %5;'>%1: </span>" "<span style='font-weight: %4;'>%2</span>").arg(QString::fromStdString(key)).arg(text).arg(std::to_string(fontSize)).arg(std::to_string(fontWeight * 1.5)).arg(std::to_string(fontWeight)));
        infoWidget->addWidget(label);
    }

    layout->addWidget(processImage(iconPath, 10, 148), 5);
    layout->addLayout(infoWidget, 9);

    return page;
}
