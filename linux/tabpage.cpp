#include "tabpage.h"
#include <qboxlayout.h>
#include <qlabel.h>
#include "json.hpp"
#include <fstream>
#include <string>
#include <vector>
#include "logger.h"
#include <optional>
#include <QPainter>
#include <QPainterPath>

namespace fs = std::filesystem;
using json = nlohmann::json;

std::string trim(const std::string& str) {
    size_t first = 0;
    while (first < str.size() && std::isspace(static_cast<unsigned char>(str[first]))) ++first;
    if (first == str.size()) return "";
    size_t last = str.size() - 1;
    while (last > first && std::isspace(static_cast<unsigned char>(str[last]))) --last;
    return str.substr(first, last - first + 1);
}

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

json getOS() {
    json result;
    std::vector<std::string> keys;
    std::ifstream file("/etc/os-release");
    std::string line;

    while (std::getline(file, line)) {
        if (!(line.empty() || line.find_first_not_of(" \t\n\r") == std::string::npos)) {
            size_t pos = line.find("=");

            if (pos != std::string::npos) {
                std::string key = line.substr(0, pos);
                std::string value = line.substr(pos + 1);
                if (value[0] == '"') value = value.substr(1);
                if (value.back() == '"') value.pop_back();
                Logger::verbose(QString("Found key: %1 (value: %2)").arg(key).arg(value));
                result[key] = value;
                keys.push_back(key);
            }
        }
    }

    Logger::print(QString("Got OS info (keys: %1)").arg(keys.size()));
    return result;
}

json getCPU() {
    json result;
    std::vector<std::string> keys;
    std::ifstream file("/proc/cpuinfo");
    std::string line;

    while (std::getline(file, line)) {
        if (!(line.empty() || line.find_first_not_of(" \t\n\r") == std::string::npos)) {
            size_t pos = line.find(":");

            if (pos != std::string::npos) {
                std::string key = trim(line.substr(0, pos));
                std::string value = trim(line.substr(pos + 1));

                if (!result.contains(key)) {
                    Logger::verbose(QString("Found key: %1 (value: %2)").arg(key).arg(value));
                    result[key] = value;
                    keys.push_back(key);
                }
            }
        }
    }

    Logger::print(QString("Got CPU info (keys: %1)").arg(keys.size()));
    return result;
}

template<typename T>
std::optional<T> atKeyOrNull(const json& j, const std::string& key) {
    if (j.contains(key)) return j.at(key).get<T>();
    return std::nullopt;
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

std::string getModel() {
    std::ifstream file("/sys/devices/virtual/dmi/id/product_name");
    std::string model;

    if (file.is_open()) {
        std::getline(file, model);
        file.close();
        return model;
    } else {
        return "Unknown model";
    }
}

QWidget* LocalTabPage::overview(QWidget* parent) {
    QWidget *page = new QWidget();
    QVBoxLayout *infoWidget = new QVBoxLayout();
    QHBoxLayout *layout = new QHBoxLayout(page);

    json results;
    json osInfo = getOS();
    json cpuInfo = getCPU();

    std::optional<std::string> iconId = atKeyOrNull<std::string>(osInfo, "LOGO");
    std::optional<fs::path> iconPath = !iconId ? std::nullopt : getIconPath(*iconId);
    Logger::print(QString("Found icon path: %1").arg(iconPath ? iconPath->string() : "none"));

    QLabel* title = new QLabel(QString::fromStdString(osInfo["PRETTY_NAME"].get<std::string>()));
    QString cpuModel = QString::fromStdString(cpuInfo["model name"].get<std::string>());
    results["Processor"] = cpuModel.toStdString();
    QFont font = title->font();

    QLabel* modelLabel = new QLabel;
    modelLabel->setTextFormat(Qt::RichText);
    modelLabel->setText(QString("<span style='font-weight: bold;'>%1</span>").arg(getModel()));

    font.setPointSize(24);
    title->setFont(font);

    infoWidget->setAlignment(Qt::AlignTop);
    infoWidget->addWidget(title);
    infoWidget->addWidget(modelLabel);

    for (auto& [key, value] : results.items()) {
        QLabel* label = new QLabel;
        QString text = QString::fromStdString(value.dump());
        if (value.is_string()) text = QString::fromStdString(value.get<std::string>());
        label->setTextFormat(Qt::RichText);
        label->setText(QString("<span>%1</span>" ": " "<span style='font-weight: bold;'>%2</span>").arg(QString::fromStdString(key)).arg(text));
        infoWidget->addWidget(label);
    }

    layout->addWidget(processImage(iconPath, 10, 148), 7);
    layout->addLayout(infoWidget, 9);

    return page;
}
