#ifndef LOGGER_H
#define LOGGER_H

#include <QString>

class Logger {
public:
    Logger();
    static void warn(const QString& input);
    static void error(const QString& input);

    static void print(const QString& input);
    static void success(const QString& input);
    static void verbose(const QString& input);
    static void raw(const QString& input);
    static void setVerbose(const bool status);
    static void enableVerbose();
    static void enableLogging();
};

#endif
