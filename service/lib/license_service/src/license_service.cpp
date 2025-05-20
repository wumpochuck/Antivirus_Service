#include "../include/license_service.h"

size_t LicenseService::license_service_WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;
    response->append((char*)contents, totalSize);
    return totalSize;
}

size_t LicenseService::license_service_HeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata) {
    size_t totalSize = size * nitems;
    std::string header(buffer, totalSize);

    // Проверяем наличие Authorization в заголовке
    if (header.find("Authorization: Bearer") != std::string::npos) {
        size_t tokenStart = header.find("Bearer") + 7; // Пропускаем "Bearer "
        std::string newAccessToken = header.substr(tokenStart);

        // Удаляем пробелы и символы новой строки вокруг токена
        newAccessToken.erase(0, newAccessToken.find_first_not_of(" \r\n"));
        newAccessToken.erase(newAccessToken.find_last_not_of(" \r\n") + 1);

        if (!newAccessToken.empty() && newAccessToken != "null") {
            AccountManager* accountManager = static_cast<AccountManager*>(userdata);
            accountManager->setJwt(newAccessToken);
            logger.Log(LogLevel::INFO, "LicenseService.cpp: updateRequest: Access token updated from headers.");
        }
    }

    return totalSize;
}

string LicenseService::activateLicenseRequest(string data) {
    // data содержит "activationCode:deviceName:macAddress"
    size_t activationCodeEnd = data.find(":");
    string activationCode = data.substr(0, activationCodeEnd);

    size_t deviceNameStart = activationCodeEnd + 1;
    size_t deviceNameEnd = data.find(":", deviceNameStart);
    string deviceName = data.substr(deviceNameStart, deviceNameEnd - deviceNameStart);

    string macAddress = data.substr(deviceNameEnd + 1);

    string jsonData = "{\"activationCode\":\"" + activationCode + "\",\"deviceName\":\"" + deviceName + "\",\"macAddress\":\"" + macAddress + "\"}";

    CURL* curl;
    CURLcode res;
    string response = "";

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (!curl) {
        logger.Log(LogLevel::ERR, "LicenseService.cpp: activateLicenseRequest: curl_easy_init() failed");
        return "Initialization failed";
    }

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // Если токен доступен, добавляем его в заголовок Authorization
    if (!accountManager.getJwt().empty()) {
        string authHeader = "Authorization: Bearer " + accountManager.getJwt();
        headers = curl_slist_append(headers, authHeader.c_str());
    }

    curl_easy_setopt(curl, CURLOPT_URL, SERVER_IP "/license/activate");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, license_service_WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, license_service_HeaderCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, this); // Передаем указатель на текущий объект

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        logger.Log(LogLevel::ERR, "LicenseService.cpp: activateLicenseRequest: curl_easy_perform() failed: " + string(curl_easy_strerror(res)));
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        return "Request failed";
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    logger.Log(LogLevel::INFO, "LicenseService.cpp: activateLicenseRequest: Response: " + response);

    // Проверяем успешность активации
    if (response.find("License successfully activated") != string::npos) {
        logger.Log(LogLevel::INFO, "LicenseService.cpp: activateLicenseRequest: Activation successful. Ticket: " + response);
        size_t ticketStart = response.find("Ticket{");
        size_t ticketEnd = response.find("}", ticketStart) + 1;
        setTicket(response.substr(ticketStart, ticketEnd - ticketStart));
        setLicenseCode(activationCode);
        return "License activated " + getTicket();
    } else {
        logger.Log(LogLevel::ERR, "LicenseService.cpp: activateLicenseRequest: Activation failed. Response: " + response);
        return response;
    }

    return response;
}

string LicenseService::licenseInfoRequest(string data) {
    // data содержит только macAddress
    string macAddress = data;

    // Получаем licenseCode из класса
    string licenseCode = getLicenseCode();
    if (licenseCode.empty()) {
        logger.Log(LogLevel::ERR, "LicenseService.cpp: licenseInfoRequest: Saved license code is empty. Trying to find by name.");
        licenseCode = "123";
        // return "License code is empty";
    }

    // Формируем JSON body
    string jsonData = "{\"macAddress\":\"" + macAddress + "\",\"licenseCode\":\"" + licenseCode + "\"}";

    CURL* curl;
    CURLcode res;
    string response = "";

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (!curl) {
        logger.Log(LogLevel::ERR, "LicenseService.cpp: licenseInfoRequest: curl_easy_init() failed");
        return "Initialization failed";
    }

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // Если токен доступен, добавляем его в заголовок Authorization
    if (!accountManager.getJwt().empty()) {
        string authHeader = "Authorization: Bearer " + accountManager.getJwt();
        headers = curl_slist_append(headers, authHeader.c_str());
    }

    curl_easy_setopt(curl, CURLOPT_URL, SERVER_IP "/license/info");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, license_service_WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, license_service_HeaderCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, this); // Передаем указатель на текущий объект

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        logger.Log(LogLevel::ERR, "LicenseService.cpp: licenseInfoRequest: curl_easy_perform() failed: " + string(curl_easy_strerror(res)));
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        return "Request failed";
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    logger.Log(LogLevel::INFO, "LicenseService.cpp: licenseInfoRequest: Response: " + response);

    // Проверяем успешность запроса
    if (response.find("License found") != string::npos) {
        logger.Log(LogLevel::INFO, "LicenseService.cpp: licenseInfoRequest: License found. Response: " + response);
        return response;
    } else {
        logger.Log(LogLevel::ERR, "LicenseService.cpp: licenseInfoRequest: Request failed. Response: " + response);
        return response;
    }

    return response;
}

string LicenseService::licenseUpdateRequest(string data) {
    // Разделяем входные данные на login, password, licenseCode и macAddress
    size_t loginEnd = data.find(":");
    string login = data.substr(0, loginEnd);

    size_t passwordStart = loginEnd + 1;
    size_t passwordEnd = data.find(":", passwordStart);
    string password = data.substr(passwordStart, passwordEnd - passwordStart);

    size_t licenseCodeStart = passwordEnd + 1;
    size_t licenseCodeEnd = data.find(":", licenseCodeStart);
    string licenseCode = data.substr(licenseCodeStart, licenseCodeEnd - licenseCodeStart);

    string macAddress = data.substr(licenseCodeEnd + 1);

    // Формируем JSON body
    string jsonData = "{\"login\":\"" + login + "\",\"password\":\"" + password + "\",\"licenseCode\":\"" + licenseCode + "\",\"macAddress\":\"" + macAddress + "\"}";

    CURL* curl;
    CURLcode res;
    string response = "";

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (!curl) {
        logger.Log(LogLevel::ERR, "LicenseService.cpp: licenseUpdateRequest: curl_easy_init() failed");
        return "Initialization failed";
    }

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // Если токен доступен, добавляем его в заголовок Authorization
    if (!accountManager.getJwt().empty()) {
        string authHeader = "Authorization: Bearer " + accountManager.getJwt();
        headers = curl_slist_append(headers, authHeader.c_str());
    }

    curl_easy_setopt(curl, CURLOPT_URL, SERVER_IP "/license/update");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, license_service_WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        logger.Log(LogLevel::ERR, "LicenseService.cpp: licenseUpdateRequest: curl_easy_perform() failed: " + string(curl_easy_strerror(res)));
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        return "Request failed";
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    logger.Log(LogLevel::INFO, "LicenseService.cpp: licenseUpdateRequest: Response: " + response);

    // Проверяем успешность обновления
    if (response.find("Successful license update") != string::npos) {
        logger.Log(LogLevel::INFO, "LicenseService.cpp: licenseUpdateRequest: Update successful. Response: " + response);
        return response;
    } else {
        logger.Log(LogLevel::ERR, "LicenseService.cpp: licenseUpdateRequest: Update failed. Response: " + response);
        return response;
    }
}







