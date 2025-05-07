#pragma once

#ifndef CONFIG_H
#define CONFIG_H

#define LOG_FILE_PATH "C:/Antivirus/logs/antivirus_service.log"
#define SERVICE_NAME _T("AntivirusService")
#define PIPE_NAME _T("\\\\.\\pipe\\AntivirusServicePipe")
#define PATH_TO_CLIENT "C:\\Antivirus\\Antivirus_Client\\antivirus\\target\\app\\bin\\app.bat"
#define WORKING_DIRECTORY "C:\\Antivirus\\Antivirus_Client\\antivirus\\target"

#define SERVER_IP "localhost"
#define SERVER_PORT 8080

extern SERVICE_STATUS g_ServiceStatus;
extern SERVICE_STATUS_HANDLE g_StatusHandle;
extern HANDLE g_ServiceStopEvent;
// extern Logger logger; // Находится в Logger.h

#endif  // CONFIG_H