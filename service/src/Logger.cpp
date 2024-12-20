#include "Logger.h"

#include <filesystem>
#include <iostream>

#include "Logger.h"

using namespace std;

string GetTimestamp() {
    auto now = chrono::system_clock::now();
    auto in_time_t = chrono::system_clock::to_time_t(now);
    tm tm_buf;
    localtime_s(&tm_buf, &in_time_t);
    stringstream ss;
    ss << put_time(&tm_buf, "%Y-%m-%d %X");
    return ss.str();
}

string LogLevelToString(LogLevel level) {
    switch (level) {
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::ERR:
            return " ERR";
        default:
            return "UNKNOWN";
    }
}

void Log(LogLevel level, const string& message) {
    string logFilePath = LOGFILE;

    if (!filesystem::exists(logFilePath)) {
        FILE* createFile;
        if (fopen_s(&createFile, logFilePath.c_str(), "w") == 0) {
            fclose(createFile);
        } else {
            cerr << "Unable to create log file" << endl;
            return;
        }
    }

    FILE* logFile;
    if (fopen_s(&logFile, logFilePath.c_str(), "a") == 0) {
        string logEntry = "[" + GetTimestamp() + "] [" + LogLevelToString(level) + "] " + message + "\n";
        fwrite(logEntry.c_str(), sizeof(char), logEntry.size(), logFile);
        fclose(logFile);
    } else {
        cerr << "Unable to open log file" << endl;
    }
}
