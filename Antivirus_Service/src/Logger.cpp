#include "Logger.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>

std::string GetTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
    return ss.str();
}

std::string LogLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::INFO: return "INFO";
        case LogLevel::ERR: return "ERR";
        default: return "UNKNOWN";
    }
}

void Log(LogLevel level, const std::string& message) {
    std::filesystem::create_directories("log");
    std::ofstream logFile(LOGFILE, std::ios_base::app);
    if (logFile.is_open()) {
        logFile << "[" << GetTimestamp() << "] [" << LogLevelToString(level) << "] " << message << std::endl;
        logFile.close();
    } else {
        std::cerr << "Unable to open log file" << std::endl;
    }
}