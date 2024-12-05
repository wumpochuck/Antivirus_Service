#define CURL_STATICLIB

#include "RequestHandler.h"
#include "Logger.h"
#include <curl/curl.h>

void SendRegisterRequest(const std::string& data, HANDLE hPipe)
{
    size_t loginEnd = data.find(':');
    size_t passwordEnd = data.find(':', loginEnd + 1);
    size_t emailEnd = data.find(':', passwordEnd + 1);

    if (loginEnd != std::string::npos && passwordEnd != std::string::npos && emailEnd == std::string::npos)
    {
        std::string login = data.substr(0, loginEnd);
        std::string password = data.substr(loginEnd + 1, passwordEnd - loginEnd - 1);
        std::string email = data.substr(passwordEnd + 1);
        std::string json = "{\"login\":\"" + login + "\",\"password\":\"" + password + "\",\"email\":\"" + email + "\"}";

        Log(LogLevel::INFO, "RequestHandler.cpp: SendRegisterRequest: JSON data: " + json);
        Log(LogLevel::INFO, "RequestHandler.cpp: SendRegisterRequest: Sending request to server");

        std::string response = SendPostRequest("http://" SERVER_IP ":8080/auth/register", json);

        DWORD bytesWritten;
        WriteFile(hPipe, response.c_str(), response.length(), &bytesWritten, NULL);

        Log(LogLevel::INFO, "RequestHandler.cpp: SendRegisterRequest: Response sent to client");
    }
    else
    {
        Log(LogLevel::ERR, "RequestHandler.cpp: SendRegisterRequest: Invalid credentials format");
    }
}

void SendLoginRequest(const std::string& data, HANDLE hPipe)
{
    size_t loginEnd = data.find(':');
    size_t passwordEnd = data.find(':', loginEnd + 1);

    if (loginEnd != std::string::npos && passwordEnd == std::string::npos)
    {
        std::string login = data.substr(0, loginEnd);
        std::string password = data.substr(loginEnd + 1, passwordEnd - loginEnd - 1);
        std::string json = "{\"login\":\"" + login + "\",\"password\":\"" + password + "\"}";

        Log(LogLevel::INFO, "RequestHandler.cpp: SendLoginRequest: JSON data: " + json);
        Log(LogLevel::INFO, "RequestHandler.cpp: SendLoginRequest: Sending request to server");

        std::string response = SendPostRequest("http://" SERVER_IP ":8080/auth/login", json);

        DWORD bytesWritten;
        WriteFile(hPipe, response.c_str(), response.length(), &bytesWritten, NULL);

        Log(LogLevel::INFO, "RequestHandler.cpp: SendLoginRequest: Response sent to client");
    }
    else
    {
        Log(LogLevel::ERR, "RequestHandler.cpp: SendLoginRequest: Invalid credentials format");
    }
}

void HandleRequest(const std::string& request, HANDLE hPipe)
{
    size_t pos = request.find(':');
    if (pos == std::string::npos)
    {
        Log(LogLevel::ERR, "RequestHandler.cpp: HandleRequest: Invalid request format");
        return;
    }

    std::string command = request.substr(0, pos);
    std::string data = request.substr(pos + 1);

    if (command == "register")
    {
        Log(LogLevel::INFO, "RequestHandler.cpp: HandleRequest: Register request");
        SendRegisterRequest(data, hPipe);
    }
    else if (command == "login")
    {
        Log(LogLevel::INFO, "RequestHandler.cpp: HandleRequest: Login request");
        SendLoginRequest(data, hPipe);
    }
    else
    {
        Log(LogLevel::ERR, "RequestHandler.cpp: HandleRequest: Unknown command");
    }
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
        headers = curl_slist_append(headers, ("Content-Length: " + std::to_string(data.length())).c_str());

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.length());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        Log(LogLevel::INFO, "RequestHandler.cpp: SendPostRequest: URL: " + url);
        Log(LogLevel::INFO, "RequestHandler.cpp: SendPostRequest: Headers: Content-Type: application/json, Content-Length: " + std::to_string(data.length()));
        Log(LogLevel::INFO, "RequestHandler.cpp: SendPostRequest: Data: " + data);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            Log(LogLevel::ERR, "RequestHandler.cpp: SendPostRequest: curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)));
        }
        else
        {
            Log(LogLevel::INFO, "RequestHandler.cpp: SendPostRequest: Response sent");
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    else {
        Log(LogLevel::ERR, "RequestHandler.cpp: SendPostRequest: curl_easy_init() failed");
    }
    curl_global_cleanup();

    return readBuffer;
}
