#pragma once
class Logger
{
public:
    Logger();
    static void warn(const std::string& input);
    static void error(const std::string& input);
    static void print(const std::string& input, bool override = false);
    static void success(const std::string& input, bool override = false);
    static void verbose(const std::string& input, bool override = false);
    static void setVerbose(const bool status);
    static void enableVerbose();
    static void enableLogging();
};

