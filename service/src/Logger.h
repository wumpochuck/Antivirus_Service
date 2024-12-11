#pragma once

#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cstdio>

enum class LogLevel {
    INFO,
    ERR
};

#define LOGFILE "C:\\antivirus_service.log"

void Log(LogLevel level, const std::string& message);
std::string LogLevelToString(LogLevel level);
std::string GetTimestamp();

#endif // LOGGER_H
