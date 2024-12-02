﻿#include "Antivirus_Service.h"
#include "Logger.h"

int _tmain (int argc, TCHAR *argv[])
{
    Log(LogLevel::INFO, "AntivirusService: Main: Entry");

    SERVICE_TABLE_ENTRY ServiceTable[] = 
    {
        {SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION) ServiceMain},
        {NULL, NULL}
    };

    if (StartServiceCtrlDispatcher (ServiceTable) == FALSE)
    {
       Log(LogLevel::ERR, "AntivirusService: Main: StartServiceCtrlDispatcher returned error");
       return GetLastError ();
    }

    Log(LogLevel::INFO, "AntivirusService: Main: Exit");
    return 0;
}

VOID WINAPI ServiceMain (DWORD argc, LPTSTR *argv)
{
    DWORD Status = E_FAIL;

    Log(LogLevel::INFO, "AntivirusService: ServiceMain: Entry");

    g_StatusHandle = RegisterServiceCtrlHandler (SERVICE_NAME, ServiceCtrlHandler);

    if (g_StatusHandle == NULL) 
    {
        Log(LogLevel::ERR, "AntivirusService: ServiceMain: RegisterServiceCtrlHandler returned error");
        Log(LogLevel::INFO, "AntivirusService: ServiceMain: Exit");
        return;
    }

    // Tell the service controller we are starting
    ZeroMemory (&g_ServiceStatus, sizeof (g_ServiceStatus));
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE) 
    {
        Log(LogLevel::ERR, "AntivirusService: ServiceMain: SetServiceStatus returned error");
    }

    /* 
     * Perform tasks necessary to start the service here
     */
    Log(LogLevel::INFO, "AntivirusService: ServiceMain: Performing Service Start Operations");

    // Create stop event to wait on later.
    g_ServiceStopEvent = CreateEvent (NULL, TRUE, FALSE, NULL);
    if (g_ServiceStopEvent == NULL) 
    {
        Log(LogLevel::ERR, "AntivirusService: ServiceMain: CreateEvent(g_ServiceStopEvent) returned error");

        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        g_ServiceStatus.dwWin32ExitCode = GetLastError();
        g_ServiceStatus.dwCheckPoint = 1;

        if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
        {
            Log(LogLevel::ERR, "AntivirusService: ServiceMain: SetServiceStatus returned error");
        }
        Log(LogLevel::INFO, "AntivirusService: ServiceMain: Exit");
        return;
    }    

    // Tell the service controller we are started
    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
        Log(LogLevel::ERR, "AntivirusService: ServiceMain: SetServiceStatus returned error");
    }

    // Start the thread that will perform the main task of the service
    HANDLE hThread = CreateThread (NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

    Log(LogLevel::INFO, "AntivirusService: ServiceMain: Waiting for Worker Thread to complete");

    // Wait until our worker thread exits effectively signaling that the service needs to stop
    WaitForSingleObject (hThread, INFINITE);
    
    Log(LogLevel::INFO, "AntivirusService: ServiceMain: Worker Thread Stop Event signaled");
    
    
    /* 
     * Perform any cleanup tasks
     */
    Log(LogLevel::INFO, "AntivirusService: ServiceMain: Performing Cleanup Operations");

    CloseHandle (g_ServiceStopEvent);

    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 3;

    if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
        Log(LogLevel::ERR, "AntivirusService: ServiceMain: SetServiceStatus returned error");
    }
    

    Log(LogLevel::INFO, "AntivirusService: ServiceMain: Exit");
    return;
}

VOID WINAPI ServiceCtrlHandler (DWORD CtrlCode)
{
    Log(LogLevel::INFO, "AntivirusService: ServiceCtrlHandler: Entry");

    switch (CtrlCode) 
    {
     case SERVICE_CONTROL_STOP :

        Log(LogLevel::INFO, "AntivirusService: ServiceCtrlHandler: SERVICE_CONTROL_STOP Request");

        if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
           break;

        /* 
         * Perform tasks necessary to stop the service here 
         */
        
        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        g_ServiceStatus.dwWin32ExitCode = 0;
        g_ServiceStatus.dwCheckPoint = 4;

        if (SetServiceStatus (g_StatusHandle, &g_ServiceStatus) == FALSE)
        {
            Log(LogLevel::ERR, "AntivirusService: ServiceCtrlHandler: SetServiceStatus returned error");
        }

        // This will signal the worker thread to start shutting down
        SetEvent (g_ServiceStopEvent);

        break;

     default:
         break;
    }

    Log(LogLevel::INFO, "AntivirusService: ServiceCtrlHandler: Exit");
}

DWORD WINAPI ServiceWorkerThread (LPVOID lpParam)
{
    Log(LogLevel::INFO, "AntivirusService: ServiceWorkerThread: Entry");
    //  Periodically check if the service has been requested to stop
    while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
    {        
        /*
            TODO: add your code here!
        */

        // Log that the service is running
        Log(LogLevel::INFO, "AntivirusService: ServiceWorkerThread: Service is running");

        // Sleeping time
        Sleep(5000);
    }

    Log(LogLevel::INFO, "AntivirusService: ServiceWorkerThread: Exit");

    return ERROR_SUCCESS;
}