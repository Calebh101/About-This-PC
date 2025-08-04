#include "global.h"
#include <string>
#include <fstream>
#include "logger.h"
#include <QString>
#include <memory>
#include <array>
#include <vector>
#include <sys/sysinfo.h>
#include <vulkan/vulkan.h>
#include <QDebug>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/fs.h>
#include <dirent.h>
#include <QLocale>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>

Global::Global() {}

const std::string Global::version = "0.0.0A";
const float Global::fontSize = 9;
const int Global::fontWeight = 400;

json helperData;

void Global::setHelperData(std::string data) {
    try {
        helperData = json::parse(data);
    } catch (const std::exception& e) {
        Logger::warn(QString("Unable to parse helper data: %1 (%2)").arg(data).arg(e.what()));
    } catch (...) {
        Logger::warn(QString("Unable to parse helper data: %1").arg(data));
    }
}

json Global::getHelperData(std::string key) {
    if (key.empty()) {
        Logger::print("Getting helper data at no key...");
        return helperData;
    } else if (helperData.contains(key)) {
        Logger::print(QString("Getting helper data at key %1...").arg(key));
        return helperData[key];
    } else {
        Logger::print(QString("Getting helper data at (invalid) key %1...").arg(key));
        json j;
        return j;
    }
}

bool Global::checkHelperData(std::string key) {
    if (helperData.empty()) return false;
    if (!key.empty() && !helperData.contains(key)) return false;
    return true;
}

json Global::getGPU() {
    json results;
    VkInstance instance;
    VkApplicationInfo appInfo{};
    VkInstanceCreateInfo createInfo{};

    results["status"] = false;
    results["gpus"] = std::vector<json>();

    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "GPU Info";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "None";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        Logger::warn("Failed to create Vulkan instance");
        return nullptr;
    }

    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        Logger::warn("No GPUs found");
        return results;
    }

    QVector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& device : devices) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(device, &props);

        VkPhysicalDeviceMemoryProperties memProps;
        vkGetPhysicalDeviceMemoryProperties(device, &memProps);

        uint64_t totalVRAM = 0;
        for (uint32_t i = 0; i < memProps.memoryHeapCount; ++i) {
            if (memProps.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) {
                totalVRAM += memProps.memoryHeaps[i].size;
            }
        }

        std::string name = props.deviceName;
        std::string type = (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU ? "iGPU" : props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ? "dGPU" : "GPU");
        int vram = totalVRAM / (1024 * 1024);

        json j;
        j["name"] = name;
        j["type"] = type;
        j["vram"] = vram;
        results["gpus"].push_back(j);
    }

    vkDestroyInstance(instance, nullptr);
    results["status"] = true;
    return results;
}

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

std::string getKernel() {
    std::ifstream file("/proc/sys/kernel/osrelease");
    std::string version;

    if (file.is_open()) {
        std::getline(file, version);
        file.close();
        return version;
    } else {
        return "unknown";
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

    file.close();
    result["kernel"] = getKernel();
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

ordered_json Global::getSupportUrls(json osInfo) {
    ordered_json urls;
    ordered_json results;

    urls["HOME_URL"] = "Homepage";
    urls["DOCUMENTATION_URL"] = "Documentation";
    urls["SUPPORT_URL"] = "Support";
    urls["BUG_REPORT_URL"] = "Report Bugs";
    urls["PRIVACY_POLICY_URL"] = "Privacy Policy";
    if (osInfo.contains("VENDOR_NAME")) urls["VENDOR_URL"] = QString("%1 Info").arg(osInfo["VENDOR_NAME"]).toStdString();
    urls["EXPERIMENT_URL"] = "Experiment Info";

    for (auto& [key, value] : urls.items()) {
        if (osInfo.contains(key)) {
            results[std::string(value)] = osInfo.at(std::string(key));
        }
    }

    results["GitHub"] = "https://github.com/Calebh101/About-This-PC";
    results["GitHub Issues"] = "https://github.com/Calebh101/About-This-PC/issues";
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

std::string Global::getComputerIconPath(std::string path) {
    return QString(":computers/images/computers/%1.png").arg(path).toStdString();
}

std::string Global::getAppIconPath() {
    return QString(":application-icon/images/application-icon.png").toStdString();
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
                icon = "monitor";
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
                icon = "monitor";
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
                icon = "laptop";
                break;

            case 13:
                result["name"] = "All-in-One";
                icon = "monitor";
                break;

            default: // 0, 1, 12, 18 - 22, 33
                result["name"] = "Computer";
                icon = "monitor";
                break;
        }
    } else {
        result["type"] = 0;
        result["name"] = "Computer";
        icon = "desktop";
    }

    result["icon"] = getComputerIconPath(icon);
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

json Global::getAllDisks() {
    json result;
    std::vector<json> drives;
    result["status"] = false;
    std::string path = "/sys/block";
    std::string startupDisk = getStartupDisk();
    DIR* dir = opendir(path.c_str());
    struct dirent* entry;

    if (!dir) {
        Logger::warn(QString("Unable to open directory: %1").arg(path));
        return result;
    }

    while ((entry = readdir(dir)) != nullptr) {
        std::string name = entry->d_name;
        std::string path = "/dev/" + name;

        if (name == "." || name == ".." || name.starts_with("loop")) continue;
        json disk;

        disk["name"] = name;
        disk["path"] = path;
        disk["data"] = getDisk(path);
        disk["startup"] = path == startupDisk;
        drives.push_back(disk);
    }

    result["status"] = true;
    result["drives"] = drives;
    Logger::print(QString("Returning %1 drive(s)...").arg(drives.size()));
    return result;
}

json Global::getDisk(std::string path) {
    json result;
    result["status"] = false;

    try {
        QString command = QString("lsblk -b -n -o SIZE,HOTPLUG -d %1").arg(path);
        Logger::print(QString("Running getDisk command of %1 for device %2").arg(command).arg(path));
        std::string processResult = run(command.toStdString());
        std::istringstream iss1(processResult);
        if (processResult.empty()) throw std::runtime_error("processResult was empty");
        long long size;
        int hotplug;
        iss1 >> size >> hotplug;
        result["bytes"] = size;
        result["external"] = (hotplug > 0 ? true : false);

        QString usedCommand = QString("df -B1 --output=source,used | grep %1").arg(path);
        std::string usedOutput = run(usedCommand.toStdString());
        if (usedOutput.empty()) throw std::runtime_error("usedOutput was empty");
        std::istringstream iss2(usedOutput);
        std::string device;
        std::string usedString;
        iss2 >> device >> usedString;
        Logger::print(QString("Used output for %1: %2 (command: %3)").arg(path).arg(QString::fromStdString(usedString)).arg(usedCommand));
        long long used = std::stoll(usedString);
        result["used"] = used;
    } catch (const std::exception& e) {
        Logger::warn(QString("getDisk(%1): %2").arg(path).arg(e.what()));
        return result;
    }

    result["status"] = true;
    return result;
}

QString Global::mmToString(double mm) {
    QLocale locale;

    if (locale.measurementSystem() == QLocale::MetricSystem) {
        return QString("%1cm").arg(std::round(mm / 10));
    } else {
        return QString("%1\"").arg(std::round(mm / 25.4));
    }
}

std::string Global::toSentenceCase(std::string input) {
    if (!input.empty() && std::islower(input[0])) input[0] = std::toupper(input[0]);
    return input;
}

std::string Global::trimDecimal(double value, int decimal) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(decimal) << value;
    std::string result = stream.str();
    return result;
}

std::vector<json> Global::getLocalIPs() {
    struct ifaddrs *ifaddr;
    struct ifaddrs *ifa;
    std::vector<json> result;
    char ip[INET_ADDRSTRLEN];
    if (getifaddrs(&ifaddr) == -1) return result;;

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == nullptr) continue;

        if (ifa->ifa_addr->sa_family == AF_INET) {
            void *addr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, addr, ip, sizeof(ip));

            if (strcmp(ifa->ifa_name, "lo") != 0) {
                std::string interface = ifa->ifa_name;
                std::string address = ip;
                json j;

                j["interface"] = interface;
                j["ip"] = address;
                result.push_back(j);
            }
        }
    }

    Logger::print(QString("Found %1 IPs").arg(result.size()));
    return result;
}

bool Global::isElevated() {
    return geteuid() == 0;
    char ip[INET_ADDRSTRLEN];
}
