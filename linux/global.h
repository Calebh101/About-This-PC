#ifndef GLOBAL_H
#define GLOBAL_H

#include "json.hpp"
#include <string>

using json = nlohmann::json;
using ordered_json = nlohmann::ordered_json;

class Global
{
public:
    Global();

    static const std::string version;
    static const float fontSize;
    static const int fontWeight;

    static std::string getModel();
    static std::string getFamily();
    static json getOS();
    static json getCPU();
    static json getMemory();
    static json getSerial();
    static json getGPU();
    static ordered_json getSupportUrls(json osInfo = getOS());
    static std::string trim(const std::string& str);
    static json getChassis();
    static std::string run(std::string cmd);
    static std::string getStartupDisk();
    static json getDisk(std::string path);
    static std::string getComputerIconPath(std::string path);
    static std::string getAppIconPath();

    template<typename T>
    static std::optional<T> atKeyOrNull(const json& j, const std::string& key) {
        if (j.contains(key)) {
            try {
                return j.at(key).get<T>();
            } catch (...) {
                return std::nullopt;
            }
        }
        return std::nullopt;
    }
};

#endif // GLOBAL_H
