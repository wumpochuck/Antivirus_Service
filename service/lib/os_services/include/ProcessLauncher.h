#ifndef PROCESSLAUNCHER_H
#define PROCESSLAUNCHER_H

#include <string>
#include <windows.h>

#include "../../logger/include/Logger.h"

class ProcessLauncher {
    public:
    // Метод для запуска процесса с передачей HANDLE через ссылку
    bool LaunchProcess(const std::string& applicationPath, const std::string& arguments, HANDLE& processHandle);

    // Метод для завершения процесса и закрытия HANDLE
    bool EndProcess(HANDLE& processHandle);
};

#endif // PROCESSLAUNCHER_H