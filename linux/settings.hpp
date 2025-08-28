#ifndef SETTINGS_H
#define SETTINGS_H

#include "logger.h"
#include "json.hpp"
#include <QWidget>
#include <fstream>
#include <QStandardPaths>

#ifdef Unsorted
#undef Unsorted
#endif
#include <QDir>

namespace fs = std::filesystem;
using json = nlohmann::json;

class Settings
{
public:
    Settings();
    static void window(QWidget* parent);
    static QWidget* page(QWidget* window);

    static QDir directory() {
        QDir directory = QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
        if (!directory.exists()) if (!directory.mkpath(".")) Logger::warn(QString("Unable to make data directory at %1!").arg(directory.absolutePath()));
        return directory;
    }

    static QString file() {
        return directory().absoluteFilePath("settings.json");;
    }

    static json& defaults() {
        static json result = [] {
            json settings;
            settings["isBeta"] = false;
            settings["checkForUpdatesAtStart"] = false;
            return settings;
        }();

        return result;
    }

    template<typename T>
    T get(QString keys) {
        bool isDefault = false;
        json output = loaded[keys.toStdString()];
        if (output.is_null()) output = defaults()[keys.toStdString()]; isDefault = true;
        Logger::print(QString("Found setting %1: %2 (default: %3) (settings: %4)").arg(keys).arg(loaded[keys.toStdString()].is_string() ? QString::fromStdString(loaded[keys.toStdString()].get<std::string>()) : QString::number(loaded[keys.toStdString()].get<double>())).arg(isDefault ? "true" : "false").arg(QString::fromStdString(this->loaded.dump())));

        if (output.is_null()) {
            Logger::warn(QString("Error with settings: Unable to load set nor default: %1 (got is_null())").arg(keys));
            return T{};
        } else {
            try {
                Logger::print(QString("Loading output of %1... (null: %2)").arg(output.get<T>()).arg(output.is_null()));
                return output.get<T>();
            } catch (json::exception e) {
                Logger::warn(QString("Error with settings: Unable to get setting in specified type: %1 (%2) (got json::exception)").arg(keys).arg(e.what()));
                return T{};
            }
        }
    }

    template<typename T>
    bool set(T value, QString keys) {
        loaded[keys.toStdString()] = value;
        std::ofstream stream(file().toStdString());

        if (!stream) {
            Logger::warn(QString("Unable to open settings file %1 to set setting %2!").arg(file(), keys));
            return false;
        }

        stream << loaded.dump() << std::endl;
        stream.close();
        Logger::print(QString("Found new setting %1: %2 (current: %3)").arg(keys).arg(loaded[keys.toStdString()].is_string() ? QString::fromStdString(loaded[keys.toStdString()].get<std::string>()) : QString::number(loaded[keys.toStdString()].get<double>())).arg(QString::fromStdString(loaded.dump())));
        return true;
    }

    bool reset() {
        json j = json::object();
        std::ofstream stream(file().toStdString());

        if (!stream) {
            Logger::warn(QString("Unable to open settings file %1 to reset settings!").arg(file()));
            return false;
        }

        stream << j.dump() << std::endl; // Empty object
        stream.close();
        return true;
    }

    template<typename T>
    bool reset(QString keys) {
        T value = T{};
        return set(value, keys);
    }

    void reload() {
        std::ifstream infile(file().toStdString());
        std::stringstream buffer;
        bool skipGood = true;
        bool status = false;
        buffer << infile.rdbuf();

        if (skipGood || infile.good()) {
            try {
                this->loaded = json::parse(buffer.str());
                status = true;
            } catch (...) {
                Logger::warn(QString("Unable to parse settings file at %1! Recovering...").arg(file()));
            }
        } else {
            Logger::warn(QString("Unable to open settings file at %1! Recovering...").arg(file()));
        }

        if (status == false) {
            Logger::print("Loading default settings...", true);
            json j = defaults();
            std::ofstream outfile(file().toStdString());
            this->loaded = j;

            if (outfile.is_open()) {
                outfile << j.dump() << std::endl;
            } else {
                Logger::print(QString("Unable to write to settings file %1: File not open").arg(file()));
            }
        }

        Logger::print("Continuing...");
    }

    json raw() {
        return loaded;
    }
private:
    json loaded;
};

class Selector
{
public:
    static QWidget* boolean(QWidget* parent, QString keys);
    static QWidget* button(QWidget* parent, QString text, QString tooltip, std::function<void()> callback);
};

#endif // SETTINGS_H
