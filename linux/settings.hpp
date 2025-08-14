#ifndef SETTINGS_H
#define SETTINGS_H

#include "logger.h"
#include "json.hpp"

using json = nlohmann::json;

class Settings
{
public:
    Settings();
    json loaded;
    static json defaults();

    template<typename T, typename... Keys>
    T get(Keys... keys) {
        json output = _get<T>(this->loaded, keys...);
        if (output.is_null()) output = _get<T>(defaults(), keys...);

        if (output.is_null()) {
            Logger::warn(QString("Error with settings: Unable to load set nor default: %1 (got is_null())").arg(joinKeys<T>(",", keys...)));
            return T{};
        } else {
            try {
                return output.get<T>();
            } catch (json::exception e) {
                Logger::warn(QString("Error with settings: Unable to get setting in specified type: %1 (%2) (got json::exception)").arg(joinKeys<T>(",", keys...)).arg(e.what()));
            }
        }
    }
private:
    template<typename T, typename... Keys>
    std::string joinKeys(const std::string& sep, const Keys&... keys) {
        std::ostringstream oss;
        ((oss << keys << sep), ...);
        std::string result = oss.str();
        if (!result.empty()) result.erase(result.size() - sep.size());
        return result;
    }

    template<typename T, typename... Keys>
    json _get(const json& settings, Keys... keys) {
        const json* current = &settings;

        for (const auto& key : {keys...}) {
            if (!current->contains(key)) return json(nullptr);
            current = &(*current)[key];
        }

        return *current;
    }

};

#endif // SETTINGS_H
