#include "Antivirus_Service.h"

#include "Logger.h"
#include "RequestHandler.h"

using namespace std;

// Main entry point for the service
int _tmain(int argc, TCHAR* argv[]) {
    Log(LogLevel::INFO, "AntivirusService.cpp: Main: Entry");

    TCHAR SERVICENAME[] = SERVICE_NAME;

    // Service table entry
    SERVICE_TABLE_ENTRY ServiceTable[] = {{SERVICENAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain}, {NULL, NULL}};

    // Start the service control dispatcher
    if (StartServiceCtrlDispatcher(ServiceTable) == FALSE) {
        Log(LogLevel::ERR,
            "AntivirusService.cpp: Main: StartServiceCtrlDispatcher returned "
            "error");
        return GetLastError();
    }

    Log(LogLevel::INFO, "AntivirusService.cpp: Main: Exit");
    return 0;
}

// Function to run JavaFX application
void runJavaFXApplication() {
    HANDLE hToken = NULL;
    DWORD dwSessionId = WTSGetActiveConsoleSessionId();

    // Get the user session token
    if (!WTSQueryUserToken(dwSessionId, &hToken)) {
        Log(LogLevel::ERR,
            "AntivirusService.cpp: runJavaFXApplication: WTSQueryUserToken "
            "failed: " +
                to_string(GetLastError()));
        return;
    }

    STARTUPINFO si = {sizeof(STARTUPINFO)};
    PROCESS_INFORMATION pi;

    // Create process in user session
    if (!CreateProcessAsUser(hToken,                 // User token
                             NULL,                   // Module name (use command line)
                             (LPSTR)PATH_TO_CLIENT,  // Command line
                             NULL,                   // Process handle not inheritable
                             NULL,                   // Thread handle not inheritable
                             FALSE,                  // Set handle inheritance to FALSE
                             CREATE_NO_WINDOW,     // Create new console window CREATE_NEW_CONSOLE CREATE_NO_WINDOW
                             NULL,                   // Use parent's environment block
                             WORKING_DIRECTORY,      // Set working directory
                             &si,                    // Pointer to STARTUPINFO structure
                             &pi)                    // Pointer to PROCESS_INFORMATION structure
    ) {
        Log(LogLevel::ERR,
            "AntivirusService.cpp: runJavaFXApplication: CreateProcessAsUser "
            "failed: " +
                to_string(GetLastError()));
        CloseHandle(hToken);
        return;
    }

    // Close process and thread handles
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    CloseHandle(hToken);
}

// Service main function
VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv) {
    DWORD Status = E_FAIL;

    Log(LogLevel::INFO, "AntivirusService.cpp: ServiceMain: Entry");

    g_StatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);

    if (g_StatusHandle == NULL) {
        Log(LogLevel::ERR,
            "AntivirusService.cpp: ServiceMain: RegisterServiceCtrlHandler "
            "returned error");
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

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE) {
        Log(LogLevel::ERR,
            "AntivirusService.cpp: ServiceMain: SetServiceStatus returned "
            "error");
    }

    // Perform tasks necessary to start the service here
    Log(LogLevel::INFO,
        "AntivirusService.cpp: ServiceMain: Performing Service Start "
        "Operations");

    // Create stop event to wait on later
    g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (g_ServiceStopEvent == NULL) {
        Log(LogLevel::ERR,
            "AntivirusService.cpp: ServiceMain: "
            "CreateEvent(g_ServiceStopEvent) returned error");

        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        g_ServiceStatus.dwWin32ExitCode = GetLastError();
        g_ServiceStatus.dwCheckPoint = 1;

        if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE) {
            Log(LogLevel::ERR,
                "AntivirusService.cpp: ServiceMain: SetServiceStatus returned "
                "error");
        }
        Log(LogLevel::INFO, "AntivirusService.cpp: ServiceMain: Exit");
        return;
    }

    // Tell the service controller we are started
    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 0;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE) {
        Log(LogLevel::ERR,
            "AntivirusService.cpp: ServiceMain: SetServiceStatus returned "
            "error");
    }

    // Start the thread that will perform the main task of the service
    HANDLE hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

    Log(LogLevel::INFO,
        "AntivirusService.cpp: ServiceMain: Waiting for Worker Thread to "
        "complete");

    // Wait until our worker thread exits effectively signaling that the service
    // needs to stop
    WaitForSingleObject(hThread, INFINITE);

    Log(LogLevel::INFO, "AntivirusService.cpp: ServiceMain: Worker Thread Stop Event signaled");

    // Perform any cleanup tasks
    Log(LogLevel::INFO, "AntivirusService.cpp: ServiceMain: Performing Cleanup Operations");

    CloseHandle(g_ServiceStopEvent);

    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 3;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE) {
        Log(LogLevel::ERR,
            "AntivirusService.cpp: ServiceMain: SetServiceStatus returned "
            "error");
    }

    Log(LogLevel::INFO, "AntivirusService.cpp: ServiceMain: Exit");
    return;
}

// Service control handler function
VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode) {
    Log(LogLevel::INFO, "AntivirusService.cpp: ServiceCtrlHandler: Entry");

    switch (CtrlCode) {
        case SERVICE_CONTROL_STOP:

            Log(LogLevel::INFO,
                "AntivirusService.cpp: ServiceCtrlHandler: "
                "SERVICE_CONTROL_STOP Request");

            if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING) break;

            // Perform tasks necessary to stop the service here
            g_ServiceStatus.dwControlsAccepted = 0;
            g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
            g_ServiceStatus.dwWin32ExitCode = 0;
            g_ServiceStatus.dwCheckPoint = 4;

            if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE) {
                Log(LogLevel::ERR,
                    "AntivirusService.cpp: ServiceCtrlHandler: "
                    "SetServiceStatus returned error");
            }

            // This will signal the worker thread to start shutting down
            SetEvent(g_ServiceStopEvent);

            break;

        default:
            break;
    }

    Log(LogLevel::INFO, "AntivirusService.cpp: ServiceCtrlHandler: Exit");
}

// Worker thread function
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam) {
    Log(LogLevel::INFO, "AntivirusService.cpp: ServiceWorkerThread: Entry");

    runJavaFXApplication();  // Start JavaFX application

    while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0) {
        SECURITY_ATTRIBUTES sa;
        SECURITY_DESCRIPTOR sd;

        // Initialize an empty security descriptor
        if (!InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION)) {
            Log(LogLevel::ERR,
                "AntivirusService.cpp: ServiceWorkerThread: "
                "InitializeSecurityDescriptor failed: " +
                    to_string(GetLastError()));
            return 1;
        }

        // Set all users to have full access
        if (!SetSecurityDescriptorDacl(&sd, TRUE, NULL, FALSE)) {
            Log(LogLevel::ERR,
                "AntivirusService.cpp: ServiceWorkerThread: "
                "SetSecurityDescriptorDacl failed: " +
                    to_string(GetLastError()));
            return 1;
        }

        sa.nLength = sizeof(sa);
        sa.lpSecurityDescriptor = &sd;
        sa.bInheritHandle = FALSE;

        // Create named pipe
        HANDLE hPipe =
            CreateNamedPipe(PIPE_NAME, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                            PIPE_UNLIMITED_INSTANCES, 512, 512, 0,
                            &sa);  // Use SECURITY_ATTRIBUTES

        if (hPipe == INVALID_HANDLE_VALUE) {
            Log(LogLevel::ERR,
                "AntivirusService.cpp: ServiceWorkerThread: CreateNamedPipe "
                "failed: " +
                    to_string(GetLastError()));
            return 1;
        }

        Log(LogLevel::INFO,
            "AntivirusService.cpp: ServiceWorkerThread: NamedPipe waiting for "
            "data");

        // Wait for client to connect
        BOOL connected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
        if (!connected) {
            Log(LogLevel::ERR,
                "AntivirusService.cpp: ServiceWorkerThread: ConnectNamedPipe "
                "failed: " +
                    to_string(GetLastError()));
            CloseHandle(hPipe);
            continue;
        }

        Log(LogLevel::INFO,
            "AntivirusService.cpp: ServiceWorkerThread: Client start data "
            "transfer to pipe");

        char buffer[512];
        DWORD bytesRead;

        // Read data from pipe
        BOOL result = ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
        if (result && bytesRead != 0) {
            buffer[bytesRead] = '\0';
            Log(LogLevel::INFO, buffer);

            string request(buffer);
            HandleRequest(request,
                          hPipe);  // Send data to server via RequestHandler.cpp
        } else {
            DWORD error = GetLastError();
            if (error == ERROR_BROKEN_PIPE) {
                Log(LogLevel::INFO,
                    "AntivirusService.cpp: ServiceWorkerThread: Client end "
                    "data transfer to pipe or pipe has broken");
            } else {
                Log(LogLevel::ERR,
                    "AntivirusService.cpp: ServiceWorkerThread: ReadFile "
                    "failed with error " +
                        to_string(error));
            }
        }

        // Close pipe handle
        CloseHandle(hPipe);
        Log(LogLevel::INFO,
            "AntivirusService.cpp: ServiceWorkerThread: Client end data "
            "transfer to pipe");
    }

    Log(LogLevel::INFO, "AntivirusService.cpp: ServiceWorkerThread: Exit");

    return ERROR_SUCCESS;
}