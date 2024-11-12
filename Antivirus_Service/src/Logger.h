#ifndef LOGGER_H
#define LOGGER_H

#include <string>

enum class LogLevel {
    INFO,
    ERR
};

#define LOGFILE "../log/antivirus_service.log"

void Log(LogLevel level, const std::string& message);

#endif // LOGGER_H