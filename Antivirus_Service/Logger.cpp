#include "Logger.h"
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cstdio>

std::string GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf;
    localtime_s(&tm_buf, &in_time_t);
    std::stringstream ss;
    ss << std::put_time(&tm_buf, "%Y-%m-%d %X");
    return ss.str();
}

std::string LogLevelToString(LogLevel level) {
    switch (level) {
    case LogLevel::INFO: return "INFO";
    case LogLevel::ERR:  return " ERR";
    default: return "UNKNOWN";
    }
}

void Log(LogLevel level, const std::string& message) {
    // Определяем путь к файлу логов
    std::string logFilePath = LOGFILE;

    // Проверяем, существует ли файл
    if (!std::filesystem::exists(logFilePath)) {
        // Если файл не существует, создаем его
        FILE* createFile;
        if (fopen_s(&createFile, logFilePath.c_str(), "w") == 0) {
            fclose(createFile);
        }
        else {
            std::cerr << "Unable to create log file" << std::endl;
            return;
        }
    }

    // Открываем файл для записи
    FILE* logFile;
    if (fopen_s(&logFile, logFilePath.c_str(), "a") == 0) {
        std::string logEntry = "[" + GetTimestamp() + "] [" + LogLevelToString(level) + "] " + message + "\n";
        fwrite(logEntry.c_str(), sizeof(char), logEntry.size(), logFile);
        fclose(logFile);
    }
    else {
        std::cerr << "Unable to open log file" << std::endl;
    }
}

