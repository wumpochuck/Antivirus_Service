#pragma once

#ifndef PIPE_SERVICE_H
#define PIPE_SERVICE_H

#include <windows.h>
#include <string>
#include "../include/Logger.h" // Assuming logger is defined here

class PipeService {
public:
    PipeService(const std::string& pipeName, Logger& logger);
    ~PipeService();

    bool Initialize();
    bool WaitForClient();
    bool ReadData(std::string& outData);
    bool WriteData(const std::string& data);

private:
    std::string pipeName;
    HANDLE hPipe;
    Logger& logger;

    bool SetupSecurityAttributes(SECURITY_ATTRIBUTES& sa, SECURITY_DESCRIPTOR& sd);
};

// extern PipeService pipeService;

#endif // PIPE_SERVICE_H