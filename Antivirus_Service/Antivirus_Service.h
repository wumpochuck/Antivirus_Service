#pragma once

#ifndef ANTIVIRUS_SERVICE_H
#define ANTIVIRUS_SERVICE_H

#include <iostream>
#include <tchar.h>
#include <ctype.h>
#include <Windows.h>

SERVICE_STATUS        g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle = NULL;
HANDLE                g_ServiceStopEvent = INVALID_HANDLE_VALUE;

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
VOID WINAPI ServiceCtrlHandler(DWORD);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);

#define SERVICE_NAME  _T("AntivirusService")

#endif // ANTIVIRUS_SERVICE_H