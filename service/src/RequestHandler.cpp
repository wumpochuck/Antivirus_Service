#define CURL_STATICLIB

#include "RequestHandler.h"
#include "Logger.h"
#include <curl/curl.h>

// Const variables for storing JWT and other

std::string global_JWT = "";
std::string global_LOGIN = "";
std::string global_TICKET = "";
std::string global_LICENSE_CODE = "";

// Methods

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

        Log(LogLevel::INFO, "RequestHandler.cpp: SendLoginRequest: Response: " + response);

        // JWT cutting
        if (response.find("JWT{Bearer ") != std::string::npos)
        {
            size_t jwtStart = response.find("JWT{Bearer ") + 11;
            size_t jwtEnd = response.find("}", jwtStart);
            global_JWT = response.substr(jwtStart, jwtEnd - jwtStart);
            global_LOGIN = login;
            response = "Registration completed";
            Log(LogLevel::INFO, "RequestHandler.cpp: SendRegisterRequest: JWT: " + global_JWT);
        }

        WriteFile(hPipe, response.c_str(), response.length(), NULL, NULL);

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

        Log(LogLevel::INFO, "RequestHandler.cpp: SendLoginRequest: Response: " + response);

        // JWT cutting
        if (response.find("JWT{Bearer ") != std::string::npos)
        {
            size_t jwtStart = response.find("JWT{Bearer ") + 11;
            size_t jwtEnd = response.find("}", jwtStart);
            global_JWT = response.substr(jwtStart, jwtEnd - jwtStart);
            global_LOGIN = login;
            response = "Login completed";
            Log(LogLevel::INFO, "RequestHandler.cpp: SendLoginRequest: JWT: " + global_JWT);
        }

        WriteFile(hPipe, response.c_str(), response.length(), NULL, NULL);

        Log(LogLevel::INFO, "RequestHandler.cpp: SendLoginRequest: Response sent to client");
    }
    else
    {
        Log(LogLevel::ERR, "RequestHandler.cpp: SendLoginRequest: Invalid credentials format");
    }
}

void SendLicenseActivateRequest(const std::string data, HANDLE hPipe){
    size_t activationCodeEnd = data.find(':');
    size_t deviceNameEnd = data.find(':', activationCodeEnd + 1);
    size_t macAddressEnd = data.find(':', deviceNameEnd + 1);

    if (activationCodeEnd != std::string::npos && deviceNameEnd != std::string::npos && macAddressEnd == std::string::npos)
    {
        std::string activationCode = data.substr(0, activationCodeEnd);
        std::string deviceName = data.substr(activationCodeEnd + 1, deviceNameEnd - activationCodeEnd - 1);
        std::string macAddress = data.substr(deviceNameEnd + 1);
        std::string json = "{\"activationCode\":\"" + activationCode + "\",\"deviceName\":\"" + deviceName + "\",\"macAddress\":\"" + macAddress + "\"}";

        Log(LogLevel::INFO, "RequestHandler.cpp: SendLicenseActivateRequest: JSON data: " + json);
        Log(LogLevel::INFO, "RequestHandler.cpp: SendLicenseActivateRequest: Sending request to server");

        std::string response = SendPostRequest("http://" SERVER_IP ":8080/license/activate", json);

        Log(LogLevel::INFO, "RequestHandler.cpp: SendLicenseActivateRequest: Response: " + response);

        // Ticket cutting
        if(response.find("Ticket{") != std::string::npos){
            size_t ticketStart = response.find("Ticket{") + 7;
            size_t ticketEnd = response.find("}", ticketStart);
            global_TICKET = "Ticket{" + response.substr(ticketStart, ticketEnd - ticketStart) + "}";
            global_LICENSE_CODE = activationCode;
            Log(LogLevel::INFO, "RequestHandler.cpp: SendLicenseActivateRequest: " + global_TICKET);
        }

        WriteFile(hPipe, response.c_str(), response.length(), NULL, NULL);

        Log(LogLevel::INFO, "RequestHandler.cpp: SendLicenseActivateRequest: Response sent to client");
    }
    else
    {
        Log(LogLevel::ERR, "RequestHandler.cpp: SendLicenseActivateRequest: Invalid credentials format");
    }
}

void SendLicenseInfoRequest(const std::string data, HANDLE hPipe){
    size_t macAddressEnd = data.find(':');
    size_t licenseCodeEnd = data.find(':', macAddressEnd + 1);

    if (macAddressEnd != std::string::npos && licenseCodeEnd == std::string::npos)
    {
        std::string macAddress = data.substr(0, macAddressEnd);
        std::string licenseCode = data.substr(macAddressEnd + 1);
        std::string json = "{\"macAddress\":\"" + macAddress + "\",\"licenseCode\":\"" + licenseCode + "\"}";

        Log(LogLevel::INFO, "RequestHandler.cpp: SendLicenseInfoRequest: JSON data: " + json);
        Log(LogLevel::INFO, "RequestHandler.cpp: SendLicenseInfoRequest: Sending request to server");

        std::string response = SendPostRequest("http://" SERVER_IP ":8080/license/info", json);

        Log(LogLevel::INFO, "RequestHandler.cpp: SendLicenseInfoRequest: Response: " + response);

        // Ticket cutting
        if(response.find("Ticket{") != std::string::npos){
            size_t ticketStart = response.find("Ticket{") + 7;
            size_t ticketEnd = response.find("}", ticketStart);
            global_TICKET = "Ticket{" + response.substr(ticketStart, ticketEnd - ticketStart) + "}";
            global_LICENSE_CODE = licenseCode;
            Log(LogLevel::INFO, "RequestHandler.cpp: SendLicenseInfoRequest: " + global_TICKET);
        }

        WriteFile(hPipe, response.c_str(), response.length(), NULL, NULL);

        Log(LogLevel::INFO, "RequestHandler.cpp: SendLicenseInfoRequest: Response sent to client");
    }
    else
    {
        Log(LogLevel::ERR, "RequestHandler.cpp: SendLicenseInfoRequest: Invalid credentials format");
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
    else if (command == "license_activation")
    {
        Log(LogLevel::INFO, "RequestHandler.cpp: HandleRequest: License activation request");
        SendLicenseActivateRequest(data, hPipe);

    }
    else if (command == "license_info"){
        Log(LogLevel::INFO, "RequestHandler.cpp: HandleRequest: License info request");
        SendLicenseInfoRequest(data, hPipe);

    }
    else if (command == "jwtcheck"){ // return login if user is auntheticated
        Log(LogLevel::INFO, "RequestHandler.cpp: HandleRequest: JWT request");
        Log(LogLevel::INFO, "RequestHandler.cpp: HandleRequest: JWT: " + global_JWT);
        std::string isHaveJWT = (global_JWT != "") ? global_LOGIN : "false";
        WriteFile(hPipe, isHaveJWT.c_str(), isHaveJWT.length(), NULL, NULL);
    }
    else if (command == "licensecheck"){
        Log(LogLevel::INFO, "RequestHandler.cpp: HandleRequest: License check request");
        Log(LogLevel::INFO, "RequestHandler.cpp: HandleRequest: License code: " + global_LICENSE_CODE);
        WriteFile(hPipe, global_LICENSE_CODE.c_str(), global_LICENSE_CODE.length(), NULL, NULL);
    }
    else if (command == "jwtclear"){ // clear jwt etc (on exit button in client clicked)
        Log(LogLevel::INFO, "RequestHandler.cpp: HandleRequest: JWT clear request");
        global_JWT = "";
        global_LOGIN = "";
        global_TICKET = "";
        global_LICENSE_CODE = "";
        WriteFile(hPipe, "JWT cleared", 11, NULL, NULL);
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

        // If token is not empty, add it to headers
        if(global_JWT != ""){
            headers = curl_slist_append(headers, ("Authorization: Bearer " + global_JWT).c_str());
        }

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
            Log(LogLevel::INFO, "RequestHandler.cpp: SendPostRequest: Response taken");
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

std::string setGetRequest(const std::string& url, const std::string& data){
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

        // If token is not empty, add it to headers
        if(global_JWT != ""){
            headers = curl_slist_append(headers, ("Authorization: Bearer " + global_JWT).c_str());
        }

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET"); // Установка метода запроса как GET
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.length());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        Log(LogLevel::INFO, "RequestHandler.cpp: setGetRequest: URL: " + url);
        Log(LogLevel::INFO, "RequestHandler.cpp: setGetRequest: Headers: Content-Type: application/json, Content-Length: " + std::to_string(data.length()));
        Log(LogLevel::INFO, "RequestHandler.cpp: setGetRequest: Data: " + data);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            Log(LogLevel::ERR, "RequestHandler.cpp: setGetRequest: curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)));
        }
        else
        {
            Log(LogLevel::INFO, "RequestHandler.cpp: setGetRequest: Response taken");
        }

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }
    else {
        Log(LogLevel::ERR, "RequestHandler.cpp: setGetRequest: curl_easy_init() failed");
    }
    curl_global_cleanup();

    return readBuffer;
}
