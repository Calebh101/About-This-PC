#include "settings.hpp"
#include "json.hpp"
#include <QString>
#include <fstream>
#include <filesystem>
#include "logger.h"

namespace fs = std::filesystem;
using json = nlohmann::json;

Settings::Settings() {
    QString directory = QString("%1/.AboutThisPC").arg(std::getenv("HOME"));
    QString settingsfile = QString("%1/%2").arg(directory, "settings.json");

    if (!fs::exists(directory.toStdString()) || !fs::is_directory(directory.toStdString())) {
        Logger::print(QString("Creating directory at %1...").arg(directory), true);
        fs::create_directory(directory.toStdString());
    }

    std::ifstream infile(settingsfile.toStdString());
    std::stringstream buffer;
    bool status = false;
    buffer << infile.rdbuf();

    if (infile.good()) {
        try {
            this->loaded = json::parse(buffer.str());
            status = true;
        } catch (...) {
            Logger::warn(QString("Unable to parse settings file! Recovering..."));
        }
    }

    if (status == false) {
        Logger::print("Loading default settings...", true);
        json j = defaults();
        this->loaded = j;

        std::ofstream outfile(settingsfile.toStdString());

        if (outfile.is_open()) {
            outfile << j.dump() << std::endl;
        } else {
            Logger::print(QString("Unable to write to settings file %1: File not open").arg(settingsfile));
        }
    }

    Logger::print("Continuing...");
}

json Settings::defaults() {
    json result;
    result["isBeta"] = false;
    result["checkForUpdatesAtStart"] = false;
    return result;
}
