#include "../include/PipeService.h"

PipeService::PipeService(const std::string& pipeName, Logger& logger)
    : pipeName(pipeName), hPipe(INVALID_HANDLE_VALUE), logger(logger) {}

PipeService::~PipeService() {
    if (hPipe != INVALID_HANDLE_VALUE) {
        CloseHandle(hPipe);
        // logger.Log(LogLevel::INFO, "PipeService.cpp: Pipe handle closed.");
    }
}

bool PipeService::SetupSecurityAttributes(SECURITY_ATTRIBUTES& sa, SECURITY_DESCRIPTOR& sd) {
    if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
        logger.Log(LogLevel::ERR, "PipeService.cpp: Failed to initialize security descriptor.");
        return false;
    }

    if (!SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE)) {
        logger.Log(LogLevel::ERR, "PipeService.cpp: Failed to set security descriptor DACL.");
        return false;
    }

    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = &sd;
    sa.bInheritHandle = FALSE;

    return true;
}

bool PipeService::Initialize() {
    SECURITY_ATTRIBUTES sa;
    SECURITY_DESCRIPTOR sd;

    if (!SetupSecurityAttributes(sa, sd)) {
        return false;
    }

    hPipe = CreateNamedPipe(
        pipeName.c_str(),
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        512,
        512,
        0,
        &sa);

    if (hPipe == INVALID_HANDLE_VALUE) {
        logger.Log(LogLevel::ERR, "PipeService.cpp: Failed to create named pipe. Error: " + std::to_string(GetLastError()));
        return false;
    }

    // logger.Log(LogLevel::INFO, "PipeService.cpp: Named pipe created successfully.");
    return true;
}

bool PipeService::WaitForClient() {
    BOOL connected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
    if (!connected) {
        logger.Log(LogLevel::ERR, "PipeService.cpp: Failed to connect to named pipe. Error: " + std::to_string(GetLastError()));
        return false;
    }

    // logger.Log(LogLevel::INFO, "PipeService.cpp: Client connected to named pipe.");
    return true;
}

bool PipeService::ReadData(std::string& data) {
    char buffer[512];
    DWORD bytesRead;

    BOOL result = ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
    if (result && bytesRead != 0) {
        buffer[bytesRead] = '\0';
        data = buffer;
        logger.Log(LogLevel::INFO, "PipeService.cpp: Received data: " + data);
        return true;
    } else {
        DWORD error = GetLastError();
        if (error == ERROR_BROKEN_PIPE) {
            logger.Log(LogLevel::ERR, "PipeService.cpp: Client disconnected before sending data.");
        } else {
            logger.Log(LogLevel::ERR, "PipeService.cpp: Failed to read from pipe. Error: " + std::to_string(error));
        }
        return false;
    }
}

bool PipeService::WriteData(const std::string& data) {
    DWORD bytesWritten;

    BOOL result = WriteFile(
        hPipe,
        data.c_str(),
        static_cast<DWORD>(data.size()),
        &bytesWritten,
        NULL);

    if (result && bytesWritten == data.size()) {
        logger.Log(LogLevel::INFO, "PipeService.cpp: Successfully wrote data to pipe: " + data);
        return true;
    } else {
        DWORD error = GetLastError();
        logger.Log(LogLevel::ERR, "PipeService.cpp: Failed to write to pipe. Error: " + std::to_string(error));
        return false;
    }
}