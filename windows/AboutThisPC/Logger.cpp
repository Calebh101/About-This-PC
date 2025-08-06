#include "pch.h"
#include "Logger.h"

#include "logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <regex>

Logger::Logger() {}
bool useVerbose = false;
bool useLogging = false;

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
    _output("VBS", input, 2, true);
}

void _output(const std::string prefix, const std::string input, const int effect = 0, const bool allEffect = false) {
    std::string effectString = "\e[" + std::to_string(effect) + "m";
    std::string output = (allEffect ? effectString : "") + prefix + " " + currentDate() + " > \e[0m" + effectString + " " + input + "\e[0m\n";
    std::regex effectPattern("\\e\[-?\d+m");

    if (IsDebuggerPresent()) {
        OutputDebugStringA(std::regex_replace(output, effectPattern, "").c_str());
    } else {
        std::cout << output << std::endl;
    }
}

void Logger::print(const std::string& input, bool override) {
    if (!useLogging && !override) return;
    _output("LOG", input);
}

void Logger::warn(const std::string& input) {
    _output("WRN", input, 33);
}

void Logger::error(const std::string& input) {
    _output("ERR", input, 31);
}

void Logger::success(const std::string& input, bool override) {
    if (!useLogging && !override) return;
    _output("SCS", input, 32);
}

void Logger::verbose(const std::string& input, bool override) {
    if ((!useLogging || !useVerbose) && !override) return;
    _verbose(input);
}

void Logger::setVerbose(const bool status) {
    if (!useLogging) return;
    if (status) _verbose("Enabling verbose...");
    useVerbose = status;
}

void Logger::enableVerbose() {
    if (!useLogging) return;
    _verbose("Enabling verbose...");
    useVerbose = true;
}

void Logger::enableLogging() {
    print("Logging enabled");
    useLogging = true;
}
