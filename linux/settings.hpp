#ifndef SETTINGS_H
#define SETTINGS_H

#include "logger.h"
#include "json.hpp"
#include <QWidget>
#include "fstream"

using json = nlohmann::json;

class Settings
{
public:
    Settings();
    json loaded;
    static QWidget* page(QWidget* parent);

    static json& defaults() {
        static json result = [] {
            json settings;
            result["isBeta"] = false;
            result["checkForUpdatesAtStart"] = false;
            return settings;
        }();

        return result;
    }

    template<typename T>
    T get(QStringList keys) {
        json output = _get<T>(this->loaded, keys);
        if (output.is_null()) output = _get<T>(defaults(), keys);

        if (output.is_null()) {
            Logger::warn(QString("Error with settings: Unable to load set nor default: %1 (got is_null())").arg(keys.join(":")));
            return T{};
        } else {
            try {
                return output.get<T>();
            } catch (json::exception e) {
                Logger::warn(QString("Error with settings: Unable to get setting in specified type: %1 (%2) (got json::exception)").arg(keys.join(":")).arg(e.what()));
                return T{};
            }
        }
    }

    template<typename T>
    bool set(T value, QStringList keys) {
        _get<T>(loaded, keys) = value;
        QString file = QString("%1/.AboutThisPC/settings.json").arg(std::getenv("HOME"));
        std::ofstream stream(file.toStdString());

        if (!stream) {
            Logger::warn(QString("Unable to open settings file %1 to set setting %2!").arg(file, keys.join(":")));
            return false;
        }

        stream << loaded.dump() << std::endl;
        stream.close();
        return true;
    }
private:
    template <typename T>
    json& _get(json& settings, QStringList keys) {
        json* current = &settings;
        for (auto& key : keys) current = &(*current)[key.toStdString()];
        return *current;
    }
};

#endif // SETTINGS_H
