#include "../include/ProcessLauncher.h"

bool ProcessLauncher::GetUserToken(HANDLE& hToken) {
    DWORD dwSessionId = 0;
    const int timeoutInMilliseconds = 120000; // 2 минуты
    const int sleepInterval = 1000; // 1 секунда
    int elapsedTime = 0;

    // Пытаемся получить идентификатор активной сессии и токен пользователя
    while (elapsedTime < timeoutInMilliseconds) {
        dwSessionId = WTSGetActiveConsoleSessionId();
        if (dwSessionId != 0xFFFFFFFF) {
            if (WTSQueryUserToken(dwSessionId, &hToken)) {
                // Успешно получили токен
                return true;
            } else {
                logger.Log(LogLevel::ERR, "ProcessLauncher.cpp: GetUserToken: WTSQueryUserToken failed, retrying... Error: " + std::to_string(GetLastError()));
            }
        } else {
            logger.Log(LogLevel::ERR, "ProcessLauncher.cpp: GetUserToken: No active console session found, retrying...");
        }

        Sleep(sleepInterval);
        elapsedTime += sleepInterval;
    }

    logger.Log(LogLevel::ERR, "ProcessLauncher.cpp: GetUserToken: Failed to obtain user token after 2 minutes.");
    return false;
}

bool ProcessLauncher::LaunchProcess(const std::string& applicationPath, const std::string& applicationWorkingPath, const std::string& arguments, HANDLE& processHandle) {
    HANDLE hToken = NULL;

    // Получаем токен пользователя
    if (!GetUserToken(hToken)) {
        return false;
    }
    

    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi = { 0 };

    std::string commandLine = applicationPath + " " + arguments;

    // Создаем процесс в пользовательской сессии
    if (!CreateProcessAsUser(
            hToken,                        // Токен пользователя
            NULL,                          // Имя модуля (используем командную строку)
            &commandLine[0],           // Командная строка
            NULL,                          // Дескриптор процесса не наследуем
            NULL,                          // Дескриптор потока не наследуем
            FALSE,                         // Устанавливаем наследование дескрипторов в FALSE
            CREATE_NEW_CONSOLE,            // Создаем новое консольное окно
            NULL,                          // Используем окружение родительского процесса
            applicationWorkingPath.c_str(),  // Рабочая директория
            &si,                           // Указатель на структуру STARTUPINFO
            &pi                            // Указатель на структуру PROCESS_INFORMATION
        )) {
        logger.Log(LogLevel::ERR, "ProcessLauncher.cpp: LaunchProcess: CreateProcessAsUser failed: " + std::to_string(GetLastError()));
        CloseHandle(hToken);
        return false;
    }

    // Передаем handle процесса через ссылку
    processHandle = pi.hProcess;

    // Закрываем ненужные дескрипторы
    CloseHandle(pi.hThread);
    CloseHandle(hToken);

    logger.Log(LogLevel::INFO, "ProcessLauncher.cpp: LaunchProcess: Process launched successfully: " + applicationPath + " " + arguments);
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

HANDLE ProcessLauncher::LaunchStarter(const std::string& starterPath, const std::string& arguments) {
    HANDLE hToken = NULL;

    if (!GetUserToken(hToken)) {
        return INVALID_HANDLE_VALUE;
    }

    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi = { 0 };

    std::string commandLine = starterPath + " " + arguments;

    if (!CreateProcessAsUser(
            hToken,
            NULL,
            &commandLine[0],
            NULL,
            NULL,
            FALSE,
            CREATE_NO_WINDOW,
            NULL,
            NULL,
            &si,
            &pi)) {
        logger.Log(LogLevel::ERR, "ProcessLauncher.cpp: LaunchStarter: CreateProcessAsUser failed: " + std::to_string(GetLastError()));
        CloseHandle(hToken);
        return INVALID_HANDLE_VALUE;
    }

    CloseHandle(pi.hThread);
    CloseHandle(hToken);

    logger.Log(LogLevel::INFO, "ProcessLauncher.cpp: LaunchStarter: Starter process launched successfully: " + starterPath + " " + arguments);
    return pi.hProcess; // Возвращаем HANDLE процесса
}