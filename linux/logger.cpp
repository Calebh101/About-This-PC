#include "logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <QString>
#include <sstream>

Logger::Logger() {}
bool useVerbose = false;

std::string currentDate() {
    auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm tm_utc;
    std::ostringstream oss;

#if defined(_WIN32) || defined(_WIN64)
    gmtime_s(&tm_utc, &now);
#else
    gmtime_r(&now, &tm_utc);
#endif

    oss << std::put_time(&tm_utc, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

void _verbose(const std::string input) {
    std::cout << "\e[2mVBS " + currentDate() + " > \e[0m\e[2m " + input + "\e[0m" << std::endl;
}

void Logger::print(const QString& input) {
    std::cout << "LOG " + currentDate() + " > \e[0m " + input.toStdString() + "\e[0m" << std::endl;
}

void Logger::warn(const QString& input) {
    std::cout << "WRN " + currentDate() + " > \e[0m\e[33m " + input.toStdString() + "\e[0m" << std::endl;
}

void Logger::error(const QString& input) {
    std::cout << "ERR " + currentDate() + " > \e[0m\e[31m " + input.toStdString() + "\e[0m" << std::endl;
}

void Logger::success(const QString& input) {
    std::cout << "SCS " + currentDate() + " > \e[0m\e[32m " + input.toStdString() + "\e[0m" << std::endl;
}

void Logger::verbose(const QString& input) {
    if (!useVerbose) return;
    _verbose(input.toStdString());
}

void Logger::setVerbose(const bool status) {
    if (status) _verbose("Enabling verbose...");
    useVerbose = status;
}

void Logger::enableVerbose() {
    _verbose("Enabling verbose...");
    useVerbose = true;
}
