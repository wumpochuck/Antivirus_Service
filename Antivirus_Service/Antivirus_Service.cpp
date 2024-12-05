#include <windows.h>
#include <string>
#include <curl/curl.h>
#include "Antivirus_Service.h"
#include "Logger.h"

#pragma comment(lib, "libcurl.lib")

#define PIPE_NAME _T("\\\\.\\pipe\\AntivirusServicePipe")
#define SERVER_IP "192.168.0.21"
#define SERVER_PORT 8080

std::string ConvertToUTF8(const std::wstring& wstr)
{
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

std::string SendPostRequest(const std::string& url, const std::string& data)
{
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (curl)
    {
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            Log(LogLevel::ERR, "AntivirusService: SendPostRequest: curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)));
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    curl_global_cleanup();

    return readBuffer;
}

int _tmain(int argc, TCHAR* argv[])
{
    Log(LogLevel::INFO, "AntivirusService: Main: Entry");

    TCHAR SERVICENAME[] = SERVICE_NAME;

    SERVICE_TABLE_ENTRY ServiceTable[] =
    {
        {SERVICENAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain},
        {NULL, NULL}
    };

    if (StartServiceCtrlDispatcher(ServiceTable) == FALSE)
    {
        Log(LogLevel::ERR, "AntivirusService: Main: StartServiceCtrlDispatcher returned error");
        return GetLastError();
    }

    Log(LogLevel::INFO, "AntivirusService: Main: Exit");
    return 0;
}

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv)
{
    DWORD Status = E_FAIL;

    Log(LogLevel::INFO, "AntivirusService: ServiceMain: Entry");

    g_StatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, ServiceCtrlHandler);

    if (g_StatusHandle == NULL)
    {
        Log(LogLevel::ERR, "AntivirusService: ServiceMain: RegisterServiceCtrlHandler returned error");
        Log(LogLevel::INFO, "AntivirusService: ServiceMain: Exit");
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
        Log(LogLevel::ERR, "AntivirusService: ServiceMain: SetServiceStatus returned error");
    }

    /*
     * Perform tasks necessary to start the service here
     */
    Log(LogLevel::INFO, "AntivirusService: ServiceMain: Performing Service Start Operations");

    // Create stop event to wait on later.
    g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (g_ServiceStopEvent == NULL)
    {
        Log(LogLevel::ERR, "AntivirusService: ServiceMain: CreateEvent(g_ServiceStopEvent) returned error");

        g_ServiceStatus.dwControlsAccepted = 0;
        g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
        g_ServiceStatus.dwWin32ExitCode = GetLastError();
        g_ServiceStatus.dwCheckPoint = 1;

        if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
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

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
        Log(LogLevel::ERR, "AntivirusService: ServiceMain: SetServiceStatus returned error");
    }

    // Start the thread that will perform the main task of the service
    HANDLE hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

    Log(LogLevel::INFO, "AntivirusService: ServiceMain: Waiting for Worker Thread to complete");

    // Wait until our worker thread exits effectively signaling that the service needs to stop
    WaitForSingleObject(hThread, INFINITE);

    Log(LogLevel::INFO, "AntivirusService: ServiceMain: Worker Thread Stop Event signaled");


    /*
     * Perform any cleanup tasks
     */
    Log(LogLevel::INFO, "AntivirusService: ServiceMain: Performing Cleanup Operations");

    CloseHandle(g_ServiceStopEvent);

    g_ServiceStatus.dwControlsAccepted = 0;
    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    g_ServiceStatus.dwWin32ExitCode = 0;
    g_ServiceStatus.dwCheckPoint = 3;

    if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
    {
        Log(LogLevel::ERR, "AntivirusService: ServiceMain: SetServiceStatus returned error");
    }


    Log(LogLevel::INFO, "AntivirusService: ServiceMain: Exit");
    return;
}

VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
    Log(LogLevel::INFO, "AntivirusService: ServiceCtrlHandler: Entry");

    switch (CtrlCode)
    {
    case SERVICE_CONTROL_STOP:

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

        if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
        {
            Log(LogLevel::ERR, "AntivirusService: ServiceCtrlHandler: SetServiceStatus returned error");
        }

        // This will signal the worker thread to start shutting down
        SetEvent(g_ServiceStopEvent);

        break;

    default:
        break;
    }

    Log(LogLevel::INFO, "AntivirusService: ServiceCtrlHandler: Exit");
}

DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
{
    Log(LogLevel::INFO, "AntivirusService: ServiceWorkerThread: Entry");

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
            Log(LogLevel::ERR, "AntivirusService: ServiceWorkerThread: CreateNamedPipe failed");
            return 1;
        }

        Log(LogLevel::INFO, "AntivirusService: ServiceWorkerThread: Waiting for client connection");

        BOOL connected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
        if (!connected)
        {
            Log(LogLevel::ERR, "AntivirusService: ServiceWorkerThread: ConnectNamedPipe failed");
            CloseHandle(hPipe);
            continue;
        }

        Log(LogLevel::INFO, "AntivirusService: ServiceWorkerThread: Client connected");

        char buffer[512];
        DWORD bytesRead;

        BOOL result = ReadFile(hPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
        if (result && bytesRead != 0)
        {
            buffer[bytesRead] = '\0';
            Log(LogLevel::INFO, buffer);

            std::string data(buffer);
            size_t pos = data.find("register:");
            if (pos != std::string::npos)
            {
                Log(LogLevel::INFO, "AntivirusService: ServiceWorkerThread: Register request");
                std::string credentials = data.substr(pos + 9);
                size_t loginEnd = credentials.find(':');
                size_t passwordEnd = credentials.find(':', loginEnd + 1);
                size_t emailEnd = credentials.find(':', passwordEnd + 1);

                Log(LogLevel::INFO, "AntivirusService: ServiceWorkerThread: Parsed credentials - loginEnd: " + std::to_string(loginEnd) + ", passwordEnd: " + std::to_string(passwordEnd) + ", emailEnd: " + std::to_string(emailEnd));

                if (loginEnd != std::string::npos && passwordEnd != std::string::npos && emailEnd != std::string::npos)
                {
                    std::string login = credentials.substr(0, loginEnd);
                    std::string password = credentials.substr(loginEnd + 1, passwordEnd - loginEnd - 1);
                    std::string email = credentials.substr(passwordEnd + 1);

                    Log(LogLevel::INFO, "AntivirusService: ServiceWorkerThread: Parsed values - login: " + login + ", password: " + password + ", email: " + email);

                    std::string json = "{\"login\":\"" + login + "\",\"password\":\"" + password + "\",\"email\":\"" + email + "\"}";

                    Log(LogLevel::INFO, "AntivirusService: ServiceWorkerThread: JSON data: " + json);

                    Log(LogLevel::INFO, "AntivirusService: ServiceWorkerThread: Sending request to server");

                    std::string response = SendPostRequest("http://" SERVER_IP ":8080/auth/register", json);

                    Log(LogLevel::INFO, "AntivirusService: ServiceWorkerThread: Response from server: " + response);

                    DWORD bytesWritten;
                    WriteFile(hPipe, response.c_str(), response.length(), &bytesWritten, NULL);

                    Log(LogLevel::INFO, "AntivirusService: ServiceWorkerThread: Response sent to client");
                }
                else
                {
                    Log(LogLevel::ERR, "AntivirusService: ServiceWorkerThread: Invalid credentials format");
                }
            }
        }
        else
        {
            DWORD error = GetLastError();
            if (error == ERROR_BROKEN_PIPE)
            {
                Log(LogLevel::INFO, "AntivirusService: ServiceWorkerThread: Client disconnected");
            }
            else
            {
                Log(LogLevel::ERR, "AntivirusService: ServiceWorkerThread: ReadFile failed with error " + std::to_string(error));
            }
        }

        CloseHandle(hPipe);
        Log(LogLevel::INFO, "AntivirusService: ServiceWorkerThread: Client disconnected");
    }

    Log(LogLevel::INFO, "AntivirusService: ServiceWorkerThread: Exit");

    return ERROR_SUCCESS;
}

