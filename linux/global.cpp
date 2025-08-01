#include "global.h"
#include <string>
#include <fstream>
#include "logger.h"
#include <QString>
#include <memory>
#include <array>
#include <vector>
#include <sys/sysinfo.h>

Global::Global() {}
const float Global::fontSize = 9;
const int Global::fontWeight = 400;

std::string Global::getModel() {
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

std::string Global::getFamily() {
    std::ifstream file("/sys/devices/virtual/dmi/id/product_family");
    std::string model;

    if (file.is_open()) {
        std::getline(file, model);
        file.close();

        int pos = model.find(' ');
        if (pos != std::string::npos) return model.substr(pos + 1);
        return model;
    } else {
        return getModel();
    }
}

json Global::getOS() {
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

json Global::getCPU() {
    std::vector<json> results;
    std::vector<std::string> processors;
    std::ifstream file("/proc/cpuinfo");
    std::string line;

    while (std::getline(file, line)) {
        if (!(line.empty() || line.find_first_not_of(" \t\n\r") == std::string::npos)) {
            size_t pos = line.find(":");

            if (pos != std::string::npos) {
                json result;
                std::string key = trim(line.substr(0, pos));
                std::string value = trim(line.substr(pos + 1));

                if (key == "model name" && !(std::find(processors.begin(), processors.end(), value) != processors.end())) {
                    processors.push_back(value);
                }

                if (!result.contains(key)) {
                    Logger::verbose(QString("Found key: %1 (value: %2)").arg(key).arg(value));
                    result[key] = value;
                    results.push_back(result);
                }
            }
        }
    }

    std::string speedString = run("lscpu | grep \"CPU min MHz\"");
    size_t speedPos = speedString.find(":");
    std::string speed = trim(speedString.substr(speedPos + 1));
    float speedNum = std::stof(speed) / 1000;

    std::string arch;
    std::string archRaw = run("uname -m");

    if (archRaw == "x86_64") {
        arch = "x64";
    } else if (archRaw == "i386" || "i686") {
        arch = "x86";
    } else if (archRaw == "aarch64") {
        arch = "ARM64";
    } else if (archRaw == "armv7l" || "armv6l") {
        arch = "ARM";
    } else {
        arch = "Unknown";
    }

    json j;
    j["results"] = results;
    j["processors"] = processors;
    j["speed"] = speedNum;
    j["arch"] = arch;
    Logger::print(QString("Got CPU info (found: %1) (speed: %2)").arg(results.size()).arg(speedNum));
    return j;
}

json Global::getMemory() {
    json results;
    struct sysinfo info;

    if(sysinfo(&info) == 0) {
        double factor = 1000;
        double gb = static_cast<double>(info.totalram) * info.mem_unit / (factor * factor * 1024);
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1) << gb;
        results["totalString"] = QString("%1 GiB").arg(oss.str()).toStdString();
        results["total"] = gb;
    }

    try {
        json dmi;
        std::string decodeString = run("sudo dmidecode -S --type 17");
        std::istringstream stream(decodeString);
        std::string line;

        while (std::getline(stream, line)) {
            if (!(line.empty() || line.find_first_not_of(" \t\n\r") == std::string::npos)) {
                size_t pos = line.find(":");

                if (pos != std::string::npos) {
                    std::string key = trim(line.substr(0, pos));
                    std::string value = trim(line.substr(pos + 1));

                    Logger::verbose(QString("Found key: %1 (value: %2)").arg(key).arg(value));
                    dmi[key] = value;
                }
            }
        }

        if (dmi.contains("Form Factor")) results["form"] = dmi["Form Factor"];
        if (dmi.contains("Type")) results["type"] = dmi["Type"];
        if (dmi.contains("Speed")) results["speed"] = dmi["Speed"];
    } catch (const std::exception& e) {
        Logger::warn(QString("Memory info error: %1").arg(e.what()));
    } catch (...) {}

    return results;
}

std::string Global::trim(const std::string& str) {
    size_t first = 0;
    while (first < str.size() && std::isspace(static_cast<unsigned char>(str[first]))) ++first;
    if (first == str.size()) return "";
    size_t last = str.size() - 1;
    while (last > first && std::isspace(static_cast<unsigned char>(str[last]))) --last;
    return str.substr(first, last - first + 1);
}

json Global::getChassis() {
    json result;
    std::ifstream file("/sys/devices/virtual/dmi/id/chassis_type");
    std::string model;
    std::string icon;

    if (file.is_open()) {
        std::getline(file, model);
        file.close();
        int type = std::stoi(model);
        result["type"] = type;

        switch (type) { // From the DMI specification at https://www.dmtf.org/sites/default/files/standards/documents/DSP0134_3.8.0.pdf
            case 3:
            case 4:
            case 7:
            case 17:
            case 23:
            case 24:
            case 25:
            case 29:
                result["name"] = "Desktop";
                icon = "desktop";
                break;

            case 5:
            case 6:
            case 15:
            case 16:
            case 26:
            case 28:
            case 34:
            case 35:
                result["name"] = "Mini PC";
                icon = "mini";
                break;

            case 8:
            case 9:
                result["name"] = "Laptop";
                icon = "laptop";
                break;


            case 10:
            case 14:
            case 31:
            case 32:
                result["name"] = "Notebook";
                icon = "laptop";
                break;
                break;

            case 11:
            case 30:
                result["name"] = "Handheld";
                icon = "handheld";
                break;

            case 13:
                result["name"] = "All-in-One";
                icon = "aio";
                break;

            default: // 0, 1, 12, 18 - 22, 33
                result["name"] = "Computer";
                icon = "desktop";
                break;
        }
    } else {
        result["type"] = 0;
        result["name"] = "Computer";
        icon = "desktop";
    }

    result["icon"] = QString(":/images/computers/%1.png").arg(icon).toStdString();
    return result;
}

std::string Global::run(std::string cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);

    if (!pipe) return "Unknown";
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) result += buffer.data();
    if (!result.empty() && result.back() == '\n') result.pop_back();
    return result;
}

std::string Global::getStartupDisk() {
    std::string command = "lsblk -no PKNAME $(df / | tail -1 | awk '{print $1}') | sed 's|^|/dev/|'";
    return run(command);
}

json Global::getDisk(std::string path) {
    std::string result = run("lsblk -J");
    return json::parse(result);
}
