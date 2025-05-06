#include "Logger.h"

#include <filesystem>
#include <iostream>
#include <sstream>
#include <chrono>
#include <iomanip>

using namespace std;

Logger::Logger(const string& logFilePath) : logFilePath(logFilePath) {
    if (!filesystem::exists(logFilePath)) {
        FILE* createFile;
        if (fopen_s(&createFile, logFilePath.c_str(), "w") == 0) {
            fclose(createFile);
        } else {
            cerr << "Unable to create log file" << endl;
        }
    }
}

void Logger::Log(LogLevel level, const string& message) {
    FILE* logFile;
    if (fopen_s(&logFile, logFilePath.c_str(), "a+") == 0) {
        string logEntry = "[" + GetTimestamp() + "] [" + LogLevelToString(level) + "] " + message + "\n";
        fwrite(logEntry.c_str(), sizeof(char), logEntry.size(), logFile);
        fclose(logFile);
    } else {
        cerr << "Unable to open log file" << endl;
    }
}

string Logger::GetTimestamp() const {
    auto now = chrono::system_clock::now();
    auto in_time_t = chrono::system_clock::to_time_t(now);
    tm tm_buf;
    localtime_s(&tm_buf, &in_time_t);
    stringstream ss;
    ss << put_time(&tm_buf, "%Y-%m-%d %X");
    return ss.str();
}

string Logger::LogLevelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::ERR:
            return " ERR";
        default:
            return "UNKNOWN";
    }
}