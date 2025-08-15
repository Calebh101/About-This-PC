#ifndef SETTINGS_H
#define SETTINGS_H

#include "logger.h"
#include "json.hpp"
#include <QWidget>
#include <fstream>

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
    json loaded;

    static void window(QWidget* parent);
    static QWidget* page(QWidget* window);
    static QDir directory;
    static QString file;

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
    T get(QStringList keys) {
        json* output = &_get(this->loaded, keys);
        if (output->is_null()) output = &_get(defaults(), keys);

        if (output->is_null()) {
            Logger::warn(QString("Error with settings: Unable to load set nor default: %1 (got is_null())").arg(keys.join(":")));
            return T{};
        } else {
            try {
                Logger::print(QString("Loading output of %1... (null: %2)").arg(output->get<T>()).arg(output->is_null()));
                return output->get<T>();
            } catch (json::exception e) {
                Logger::warn(QString("Error with settings: Unable to get setting in specified type: %1 (%2) (got json::exception)").arg(keys.join(":")).arg(e.what()));
                return T{};
            }
        }
    }

    template<typename T>
    bool set(T value, QStringList keys) {
        _get(loaded, keys) = value;
        std::ofstream stream(file.toStdString());

        if (!stream) {
            Logger::warn(QString("Unable to open settings file %1 to set setting %2!").arg(file, keys.join(":")));
            return false;
        }

        stream << loaded.dump() << std::endl;
        stream.close();
        return true;
    }

    bool reset() {
        json j = json::object();
        std::ofstream stream(file.toStdString());

        if (!stream) {
            Logger::warn(QString("Unable to open settings file %1 to reset settings!").arg(file));
            return false;
        }

        stream << j.dump() << std::endl; // Empty object
        stream.close();
        reload();
        return true;
    }

    template<typename T>
    bool reset(QStringList keys) {
        T value = T{};
        return set(value, keys);
    }

    void reload() {
        std::ifstream infile(file.toStdString());
        std::stringstream buffer;
        bool status = false;
        buffer << infile.rdbuf();

        if (infile.good()) {
            try {
                this->loaded = json::parse(buffer.str());
                status = true;
            } catch (...) {
                Logger::warn(QString("Unable to parse settings file at %1! Recovering...").arg(file));
            }
        } else {
            Logger::warn(QString("Unable to open settings file at %1! Recovering...").arg(file));
        }

        if (status == false) {
            Logger::print("Loading default settings...", true);
            json j = defaults();
            std::ofstream outfile(file.toStdString());
            this->loaded = j;

            if (outfile.is_open()) {
                outfile << j.dump() << std::endl;
            } else {
                Logger::print(QString("Unable to write to settings file %1: File not open").arg(file));
            }
        }

        Logger::print("Continuing...");
    }
private:
    json& _get(json& settings, QStringList keys) {
        json* current = &settings;
        for (QString& key : keys) current = &(*current)[key.toStdString()];
        return *current;
    }
};

class Selector
{
public:
    static QWidget* boolean(QWidget* parent, QStringList keys);
    static QWidget* button(QWidget* parent, QString text, QString tooltip, std::function<void()> callback);
};

#endif // SETTINGS_H
