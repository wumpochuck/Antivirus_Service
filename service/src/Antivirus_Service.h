#pragma once

#ifndef ANTIVIRUS_SERVICE_H
#define ANTIVIRUS_SERVICE_H

#include <Windows.h>
#include <WtsApi32.h>
#include <ctype.h>
#include <tchar.h>

#include <iostream>
#include <string>

#pragma comment(lib, "Wtsapi32.lib")

SERVICE_STATUS g_ServiceStatus = {0};
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE g_ServiceStopEvent = INVALID_HANDLE_VALUE;

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
VOID WINAPI ServiceCtrlHandler(DWORD);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);
void runJavaFXApplication();

#define SERVICE_NAME _T("AntivirusService")
#define PIPE_NAME _T("\\\\.\\pipe\\AntivirusServicePipe")
#define PATH_TO_CLIENT \
    "D:/Github/Antivirus_Client/antivirus/target/app/bin/app.bat"  // C:/Antivirus_Client/target/app/bin/app.bat
#define WORKING_DIRECTORY \
    "D:/Github/Antivirus_Client/antivirus/target"  // C:/Antivirus_Client/target
// #define PATH_TO_JDK_LIB L"C:\\javafx-sdk-23.0.1\\lib"

// IMPORTANT:
/*
 * Data from client should be: <operation>:<data>
 * Example: register:login:password:email
 * Data to client should be:
 * Status: <status>
 * Body: <body>
 */

#endif  // ANTIVIRUS_SERVICE_H