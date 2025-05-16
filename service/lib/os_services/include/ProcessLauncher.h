#ifndef PROCESSLAUNCHER_H
#define PROCESSLAUNCHER_H

#include <string>
#include <windows.h>
#include <WtsApi32.h>

#include "../../logger/include/Logger.h"

class ProcessLauncher {
    public:
    // Метод для запуска процесса с передачей HANDLE через ссылку
    bool LaunchProcess(const std::string& applicationPath, const std::string& applicationWorkingPath, const std::string& arguments, HANDLE& processHandle);
 
    // Метод для завершения процесса и закрытия HANDLE
    bool EndProcess(HANDLE& processHandle);

    HANDLE LaunchStarter(const std::string& starterPath, const std::string& arguments);

    bool GetUserToken(HANDLE& hToken);
};

extern ProcessLauncher processLauncher;

#endif // PROCESSLAUNCHER_H