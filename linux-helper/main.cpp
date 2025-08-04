#include <QCoreApplication>
#include "json.hpp"
#include <iostream>
#include <string>
#include <fstream>
#include <QString>
#include <memory>
#include <array>
#include <sys/sysinfo.h>
#include <vulkan/vulkan.h>
#include <QDebug>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <dirent.h>
#include <QLocale>

using json = nlohmann::json;

std::string run(std::string cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);

    if (!pipe) return "Unknown";
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) result += buffer.data();
    if (!result.empty() && result.back() == '\n') result.pop_back();
    return result;
}

std::string trim(const std::string& str) {
    size_t first = 0;
    while (first < str.size() && std::isspace(static_cast<unsigned char>(str[first]))) ++first;
    if (first == str.size()) return "";
    size_t last = str.size() - 1;
    while (last > first && std::isspace(static_cast<unsigned char>(str[last]))) --last;
    return str.substr(first, last - first + 1);
}

json getSerial() {
    json results;
    std::ifstream file("/sys/class/dmi/id/product_serial");
    std::string serial;

    if (file.is_open()) {
        std::getline(file, serial);
        file.close();
        results["serial"] = serial;
    }

    file.close();
    return results;
}

json getMemory() {
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
        std::string decodeString = run("dmidecode --type 17");
        std::istringstream stream(decodeString);
        std::string line;

        while (std::getline(stream, line)) {
            if (!(line.empty() || line.find_first_not_of(" \t\n\r") == std::string::npos)) {
                size_t pos = line.find(":");

                if (pos != std::string::npos) {
                    std::string key = trim(line.substr(0, pos));
                    std::string value = trim(line.substr(pos + 1));

                    dmi[key] = value;
                }
            }
        }

        if (dmi.contains("Form Factor")) results["form"] = dmi["Form Factor"];
        if (dmi.contains("Type")) results["type"] = dmi["Type"];
        if (dmi.contains("Speed")) results["speed"] = dmi["Speed"];
    } catch (...) {}

    return results;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    json results;

    results["serial"] = getSerial();
    results["memory"] = getMemory();

    std::cout << results.dump() << std::endl;
    return 0;
}
