#include "Antivirus_Service.h"
#include "Logger.h"
#include "RequestHandler.h"

int _tmain(int argc, TCHAR* argv[])
{
    Log(LogLevel::INFO, "AntivirusService.cpp: Main: Entry");

    TCHAR SERVICENAME[] = SERVICE_NAME;

    SERVICE_TABLE_ENTRY ServiceTable[] =
    {
        {SERVICENAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain},
        {NULL, NULL}
    };

    if (StartServiceCtrlDispatcher(ServiceTable) == FALSE)
    {
        Log(LogLevel::ERR, "AntivirusService.cpp: Main: StartServiceCtrlDispatcher returned error");
        return GetLastError();
    }

    Log(LogLevel::INFO, "AntivirusService.cpp: Main: Exit");
    return 0;
}
/*
// Повышения уровня доступа для запуска JavaFX приложения
bool EnablePrivilege(LPCWSTR privilegeName) {
    HANDLE token;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
        std::wcerr << L"Failed to open process token. Error: " << GetLastError() << std::endl;
        return false;
    }

    TOKEN_PRIVILEGES tp;
    LUID luid;
    if (!LookupPrivilegeValueW(NULL, privilegeName, &luid)) {
        DWORD error = GetLastError();
        std::wcerr << L"Failed to lookup privilege value. Error: " << error << std::endl;
        Log(LogLevel::ERR, "Failed to lookup privilege value. Error: " + std::to_string(error));
        CloseHandle(token);
        return false;
    }

    tp.PrivilegeCount = 1;
    tp.Privileges[0].Luid = luid;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(token, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), NULL, NULL)) {
        DWORD error = GetLastError();
        std::wcerr << L"Failed to adjust token privileges. Error: " << error << std::endl;
        Log(LogLevel::ERR, "Failed to adjust token privileges. Error: " + std::to_string(error));
        CloseHandle(token);
        return false;
    }

    CloseHandle(token);
    return GetLastError() == ERROR_SUCCESS;
}

// Запуск JavaFX приложения
void StartJavaFXClient() {
    if (!EnablePrivilege((LPCWSTR)SE_TCB_NAME)) {
        Log(LogLevel::ERR, "Failed to enable SE_TCB_NAME privilege");
        return;
    }

    DWORD sessionId = WTSGetActiveConsoleSessionId();
    HANDLE token = NULL;

    if (WTSQueryUserToken(sessionId, &token)) {
        STARTUPINFOW si = { sizeof(STARTUPINFOW) };
        si.lpDesktop = const_cast<LPWSTR>(L"winsta0\\default");
        PROCESS_INFORMATION pi = {};

        std::wstring command = L"java --module-path \"" + std::wstring(PATH_TO_JDK_LIB) + L"\" --add-modules javafx.controls,javafx.fxml -jar \"" + std::wstring(PATH_TO_CLIENT) + L"\"";

        if (CreateProcessAsUserW(
            token,
            NULL,
            &command[0],
            NULL,
            NULL,
            FALSE,
            CREATE_NO_WINDOW,
            NULL,
            NULL,
            &si,
            &pi)) {
            Log(LogLevel::INFO, "JavaFX client started");
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        else {
            Log(LogLevel::ERR, "Failed to start JavaFX client");
        }

        CloseHandle(token);
    }
    else {
        Log(LogLevel::ERR, "Failed to query user token");
    }
}
*/
VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv)
{
    DWORD Status = E_FAIL;

    Log(LogLevel::INFO, "AntivirusService.cpp: ServiceMain: Entry");

    g_StatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);

    if (g_StatusHandle == NULL)
    {
        Log(LogLevel::ERR, "AntivirusService.cpp: ServiceMain: RegisterServiceCtrlHandler returned error");
        Log(LogLevel::INFO, "AntivirusService.cpp: ServiceMain: Exit");
        return;
    }

    // Tell the service controller we are starting
    ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwServiceSpecificExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
        Log(LogLevel::ERR, "AntivirusService.cpp: ServiceMain: SetServiceStatus returned error");
    }

    /*
     * Perform tasks necessary to start the service here
     */
    Log(LogLevel::INFO, "AntivirusService.cpp: ServiceMain: Performing Service Start Operations");

    // Create stop event to wait on later.
    g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (g_ServiceStopEvent == NULL)
    {
        Log(LogLevel::ERR, "AntivirusService.cpp: ServiceMain: CreateEvent(g_ServiceStopEvent) returned error");

        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        g_ServiceStatus.dwWin32ExitCode = GetLastError();
        g_ServiceStatus.dwCheckPoint = 1;

        if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
        {
            Log(LogLevel::ERR, "AntivirusService.cpp: ServiceMain: SetServiceStatus returned error");
        }
        Log(LogLevel::INFO, "AntivirusService.cpp: ServiceMain: Exit");
        return;
    }

    // Tell the service controller we are started
    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
        Log(LogLevel::ERR, "AntivirusService.cpp: ServiceMain: SetServiceStatus returned error");
    }

    // Start the thread that will perform the main task of the service
    HANDLE hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

    Log(LogLevel::INFO, "AntivirusService.cpp: ServiceMain: Waiting for Worker Thread to complete");

    // Wait until our worker thread exits effectively signaling that the service needs to stop
    WaitForSingleObject(hThread, INFINITE);

    Log(LogLevel::INFO, "AntivirusService.cpp: ServiceMain: Worker Thread Stop Event signaled");


    /*
     * Perform any cleanup tasks
     */
    Log(LogLevel::INFO, "AntivirusService.cpp: ServiceMain: Performing Cleanup Operations");

    CloseHandle(g_ServiceStopEvent);

    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 3;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
        Log(LogLevel::ERR, "AntivirusService.cpp: ServiceMain: SetServiceStatus returned error");
    }


    Log(LogLevel::INFO, "AntivirusService.cpp: ServiceMain: Exit");
    return;
}

VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
    Log(LogLevel::INFO, "AntivirusService.cpp: ServiceCtrlHandler: Entry");

    switch (CtrlCode)
    {
    case SERVICE_CONTROL_STOP:

        Log(LogLevel::INFO, "AntivirusService.cpp: ServiceCtrlHandler: SERVICE_CONTROL_STOP Request");

        if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING)
            break;

        /*
         * Perform tasks necessary to stop the service here
         */

        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        g_ServiceStatus.dwWin32ExitCode = 0;
        g_ServiceStatus.dwCheckPoint = 4;

        if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
        {
            Log(LogLevel::ERR, "AntivirusService.cpp: ServiceCtrlHandler: SetServiceStatus returned error");
        }

        // This will signal the worker thread to start shutting down
        SetEvent(g_ServiceStopEvent);

        break;

    default:
        break;
    }

    Log(LogLevel::INFO, "AntivirusService.cpp: ServiceCtrlHandler: Exit");
}

DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
{
    Log(LogLevel::INFO, "AntivirusService.cpp: ServiceWorkerThread: Entry");

    // StartJavaFXClient(); // Запуск javafx приложения

    while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
    {
        HANDLE hPipe = CreateNamedPipe(
            PIPE_NAME,
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            512,
            512,
            0,
            NULL);

        if (hPipe == INVALID_HANDLE_VALUE)
        {
            Log(LogLevel::ERR, "AntivirusService.cpp: ServiceWorkerThread: CreateNamedPipe failed");
            return 1;
        }

        Log(LogLevel::INFO, "AntivirusService.cpp: ServiceWorkerThread: NamedPipe waiting for data");

        BOOL connected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
        if (!connected)
        {
            Log(LogLevel::ERR, "AntivirusService.cpp: ServiceWorkerThread: ConnectNamedPipe failed");
            CloseHandle(hPipe);
            continue;
        }

        Log(LogLevel::INFO, "AntivirusService.cpp: ServiceWorkerThread: Client start data transfer to pipe");

        char buffer[512];
        DWORD bytesRead;

        BOOL result = ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
        if (result && bytesRead != 0)
        {
            buffer[bytesRead] = '\0';
            Log(LogLevel::INFO, buffer);

            std::string request(buffer);
            HandleRequest(request, hPipe); // Send data to server via RequestHandler.cpp
        }
        else
        {
            DWORD error = GetLastError();
            if (error == ERROR_BROKEN_PIPE)
            {
                Log(LogLevel::INFO, "AntivirusService.cpp: ServiceWorkerThread: Client end data transfer to pipe or pipe has broken");
            }
            else
            {
                Log(LogLevel::ERR, "AntivirusService.cpp: ServiceWorkerThread: ReadFile failed with error " + std::to_string(error));
            }
        }

        CloseHandle(hPipe);
        Log(LogLevel::INFO, "AntivirusService.cpp: ServiceWorkerThread: Client end data transfer to pipe");
    }

    Log(LogLevel::INFO, "AntivirusService.cpp: ServiceWorkerThread: Exit");

    return ERROR_SUCCESS;
}
