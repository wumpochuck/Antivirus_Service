#pragma once

#ifndef ANTIVIRUS_SERVICE_H
#define ANTIVIRUS_SERVICE_H

#include <iostream>
#include <tchar.h>
#include <ctype.h>
#include <Windows.h>
#include <string>
#include <curl/curl.h>

SERVICE_STATUS        g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
VOID WINAPI ServiceCtrlHandler(DWORD);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);
std::string ConvertToUTF8(const std::wstring& wstr);


#define SERVICE_NAME  _T("AntivirusService")
#define PIPE_NAME _T("\\\\.\\pipe\\AntivirusServicePipe")

// IMPORTANT:
/*
 * Data from client should be: <operation>:<data>
 * Example: register:login:password:email
 * Data to client should be: <response code>:<response>
 */

#endif // ANTIVIRUS_SERVICE_H