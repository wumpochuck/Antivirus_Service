#pragma once

#ifndef LOGGER_H
#define LOGGER_H

#include <string>

enum class LogLevel {
    INFO,
    ERR
};

class Logger {
public:
    explicit Logger(const std::string& logFilePath);
    void Log(LogLevel level, const std::string& message);

private:
    std::string logFilePath;

    std::string GetTimestamp() const;
    std::string LogLevelToString(LogLevel level) const;
};

extern Logger logger;

#endif  // LOGGER_H