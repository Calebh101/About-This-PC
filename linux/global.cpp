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

Global::Global() {}
const float Global::fontSize = 9;
const int Global::fontWeight = 400;

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
        std::string decodeString = run("dmidecode --type 17");
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

json Global::getSerial() {
    json results;
    std::ifstream file("/sys/class/dmi/id/product_serial");
    std::string serial;

    if (file.is_open()) {
        std::getline(file, serial);
        file.close();
        results["serial"] = serial;
    }

    Logger::print(QString("Found serial: %1").arg(results.contains("serial") ? (results["serial"].is_string() ? "string" : "unknown") : "none"));
    file.close();
    return results;
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
    json result;
    result["status"] = false;

    try {
        QString command = QString("lsblk -b -n -o SIZE -d %1").arg(path);
        Logger::print(QString("Running getDisk command of %1 for device %2").arg(command).arg(path));
        std::string processResult = run(command.toStdString());
        Logger::print(QString("getDisk(%1): %2").arg(path).arg(processResult));
        long long size = std::stoll(processResult);
        result["bytes"] = size;
        result["status"] = true;
    } catch (const char* e) {
        Logger::warn(QString("getDisk(%1): %2").arg(path).arg(e));
    }

    return result;
}
