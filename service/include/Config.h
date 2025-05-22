#pragma once

#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>
#include <string>
#include <Windows.h>
#include <WtsApi32.h>
#include <ctype.h>
#include <tchar.h>
#include <cstddef>

#include <chrono>
#include <thread>
#include <ctime>

#define LOG_FILE_PATH "C:/Antivirus/logs/antivirus_service.log"
#define SERVICE_NAME _T("AntivirusService")
#define PIPE_NAME _T("\\\\.\\pipe\\AntivirusServicePipe")
#define PATH_TO_CLIENT "C:/Antivirus/Antivirus_Client/antivirus/target/app/bin/app.bat"
#define PATH_TO_CLIENT_DIR "C:/Antivirus/Antivirus_Client/antivirus/target"

#define PATH_TO_STARTER "C:/Antivirus/Antivirus_Service/service/build/Release/Starter.exe"

#define SERVER_IP "http://localhost:8080"

extern SERVICE_STATUS g_ServiceStatus;
extern SERVICE_STATUS_HANDLE g_StatusHandle;
extern HANDLE g_ServiceStopEvent;

extern HANDLE ClientHandle;
// extern Logger logger; // Находится в Logger.h
// extern AccountManager accountManager; // Находится в account_manager.h

#endif  // CONFIG_H