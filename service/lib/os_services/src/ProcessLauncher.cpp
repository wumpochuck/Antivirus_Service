#include "../include/ProcessLauncher.h"

bool ProcessLauncher::LaunchProcess(const std::string& applicationPath, const std::string& arguments, HANDLE& processHandle) {
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi = { 0 };

    std::string commandLine = applicationPath + " " + arguments;

    if (!CreateProcess(
            NULL, 
            &commandLine[0], // Command line
            NULL,            // Process handle not inheritable
            NULL,            // Thread handle not inheritable
            FALSE,           // Set handle inheritance to FALSE
            0,               // No creation flags
            NULL,            // Use parent's environment block
            NULL,            // Use parent's starting directory 
            &si,             // Pointer to STARTUPINFO structure
            &pi              // Pointer to PROCESS_INFORMATION structure
        )) {
        logger.Log(LogLevel::ERR, "Failed to create process. Error: " + std::to_string(GetLastError()));
        return false;
    }

    // Передаем handle процесса через ссылку
    processHandle = pi.hProcess;

    // Закрываем handle потока, так как он больше не нужен
    CloseHandle(pi.hThread);

    logger.Log(LogLevel::INFO, "Process launched successfully: " + applicationPath + " " + arguments);
    return true;
}

bool ProcessLauncher::EndProcess(HANDLE& processHandle) {
    if (!processHandle || processHandle == INVALID_HANDLE_VALUE) {
        logger.Log(LogLevel::ERR, "Invalid process handle provided for termination.");
        return false;
    }

    // Завершаем процесс
    if (!::TerminateProcess(processHandle, 0)) {
        logger.Log(LogLevel::ERR, "Failed to terminate process. Error: " + std::to_string(GetLastError()));
        return false;
    }

    logger.Log(LogLevel::INFO, "Process terminated successfully.");

    // Закрываем handle процесса
    if (!CloseHandle(processHandle)) {
        logger.Log(LogLevel::ERR, "Failed to close process handle. Error: " + std::to_string(GetLastError()));
        return false;
    }

    // Обнуляем handle после закрытия
    processHandle = NULL;

    return true;
}