#include "../include/account_manager.h"

size_t AccountManager::account_manager_WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response) {
    size_t totalSize = size * nmemb;
    response->append((char*)contents, totalSize);
    return totalSize;
}

size_t AccountManager::account_manager_HeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata) {
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
            logger.Log(LogLevel::INFO, "AccountManager.cpp: updateRequest: Access token updated from headers.");
        }
    }

    return totalSize;
}

string AccountManager::loginRequest(string data){

    // data contains "login:password"
    string login = data.substr(0, data.find(":"));
    string password = data.substr(data.find(":") + 1);
    string jsonData = "{\"login\":\"" + login + "\",\"password\":\"" + password + "\"}";

    CURL* curl;
    CURLcode res;
    string response = "";

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (!curl) {
        logger.Log(LogLevel::ERR, "AntivirusService.cpp: getRequest: curl_easy_init() failed");
        return response;
    }

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, ("Content-Length: " + std::to_string(jsonData.length())).c_str());

    curl_easy_setopt(curl, CURLOPT_URL, SERVER_IP "/auth/login");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, jsonData.length());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, account_manager_WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    res = curl_easy_perform(curl);

    if(res != CURLE_OK) {
        logger.Log(LogLevel::ERR, "AccountManager.cpp: loginRequest: curl_easy_perform() failed: " + string(curl_easy_strerror(res)));
    }

    curl_easy_cleanup(curl);

    logger.Log(LogLevel::INFO, "AccountManager.cpp: loginRequest: Response: " + response);

    size_t accessTokenPos = response.find("\"accessToken\":");
    size_t errorPos = response.find("\"error\":");

    if (accessTokenPos != string::npos) {
        size_t tokenStart = response.find(":", accessTokenPos) + 1;
        size_t tokenEnd = response.find(",", tokenStart);
    
        string accessToken = response.substr(tokenStart, tokenEnd - tokenStart);
    
        logger.Log(LogLevel::INFO, "AccountManager.cpp: loginRequest: Access token found: " + accessToken);
    
        if (!accessToken.empty() && accessToken.front() == '\"' && accessToken.back() == '\"') {
            accessToken = accessToken.substr(1, accessToken.size() - 2);
        }
    
        if (!accessToken.empty() && accessToken != "null") {
            setJwt(accessToken);
            setLogin(login);
            setPassword(password);
            logger.Log(LogLevel::INFO, "AccountManager.cpp: loginRequest: Login successful, token saved.");
            return "Login completed";
        }
    }

    if (errorPos != string::npos) {
        size_t errorStart = response.find(":", errorPos) + 1;
        size_t errorEnd = response.find(",", errorStart);
    
        string error = response.substr(errorStart, errorEnd - errorStart);
    
        logger.Log(LogLevel::INFO, "AccountManager.cpp: loginRequest: Error found: " + error);
    
        if (!error.empty() && error.front() == '\"' && error.back() == '\"') {
            error = error.substr(1, error.size() - 2);
        }
    
        if (!error.empty() && error != "null") {
            logger.Log(LogLevel::ERR, "AccountManager.cpp: loginRequest: Server returned error: " + error);
            return error;
        }
    }

    logger.Log(LogLevel::ERR, "AccountManager.cpp: loginRequest: No access token or error found in response.");

    return response;
}

string AccountManager::registerRequest(string data) {

    // data contains "login:password:email"
    string login = data.substr(0, data.find(":"));
    size_t passwordStart = data.find(":") + 1;
    size_t passwordEnd = data.find(":", passwordStart);
    string password = data.substr(passwordStart, passwordEnd - passwordStart);
    string email = data.substr(passwordEnd + 1);
    string jsonData = "{\"login\":\"" + login + "\",\"password\":\"" + password + "\",\"email\":\"" + email + "\"}";

    CURL* curl;
    CURLcode res;
    string response = "";

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (!curl) {
        logger.Log(LogLevel::ERR, "AccountManager.cpp: registerRequest: curl_easy_init() failed");
        return "Initialization failed";
    }

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_URL, SERVER_IP "/auth/register");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, account_manager_WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        logger.Log(LogLevel::ERR, "AccountManager.cpp: registerRequest: curl_easy_perform() failed: " + string(curl_easy_strerror(res)));
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    logger.Log(LogLevel::INFO, "AccountManager.cpp: registerRequest: Response: " + response);

    size_t accessTokenPos = response.find("\"accessToken\":");
    size_t errorPos = response.find("\"error\":");

    if (accessTokenPos != string::npos) {
        size_t tokenStart = response.find(":", accessTokenPos) + 1;
        size_t tokenEnd = response.find(",", tokenStart);

        string accessToken = response.substr(tokenStart, tokenEnd - tokenStart);

        logger.Log(LogLevel::INFO, "AccountManager.cpp: registerRequest: Access token found: " + accessToken);

        if (!accessToken.empty() && accessToken.front() == '\"' && accessToken.back() == '\"') {
            accessToken = accessToken.substr(1, accessToken.size() - 2);
        }

        if (!accessToken.empty() && accessToken != "null") {
            setJwt(accessToken);
            setLogin(login);
            setPassword(password);
            logger.Log(LogLevel::INFO, "AccountManager.cpp: registerRequest: Registration successful, token saved.");
            return "Registration completed";
        }
    }

    if (errorPos != string::npos) {
        size_t errorStart = response.find(":", errorPos) + 1;
        size_t errorEnd = response.find(",", errorStart);

        string error = response.substr(errorStart, errorEnd - errorStart);

        logger.Log(LogLevel::INFO, "AccountManager.cpp: registerRequest: Error found: " + error);

        if (!error.empty() && error.front() == '\"' && error.back() == '\"') {
            error = error.substr(1, error.size() - 2);
        }

        if (!error.empty() && error != "null") {
            logger.Log(LogLevel::ERR, "AccountManager.cpp: registerRequest: Server returned error: " + error);
            return error;
        }
    }

    logger.Log(LogLevel::ERR, "AccountManager.cpp: registerRequest: No access token or error found in response.");
    return response;
}

string AccountManager::logoutRequest() {
    if (jwt.empty()) {
        logger.Log(LogLevel::ERR, "AccountManager.cpp: logoutRequest: No token available for logout.");
        return "No token available";
    }

    CURL* curl;
    CURLcode res;
    string response = "";

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (!curl) {
        logger.Log(LogLevel::ERR, "AccountManager.cpp: logoutRequest: curl_easy_init() failed");
        return "Initialization failed";
    }

    struct curl_slist* headers = NULL;
    string authHeader = "Authorization: Bearer " + getJwt();
    headers = curl_slist_append(headers, authHeader.c_str());

    curl_easy_setopt(curl, CURLOPT_URL, SERVER_IP "/auth/logout");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POST, 1L); // Указываем, что это POST-запрос
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, account_manager_WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        logger.Log(LogLevel::ERR, "AccountManager.cpp: logoutRequest: curl_easy_perform() failed: " + string(curl_easy_strerror(res)));
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        return "Request failed";
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    logger.Log(LogLevel::INFO, "AccountManager.cpp: logoutRequest: Response: " + response);

    if (response.find("Successful") != string::npos) {
        logger.Log(LogLevel::INFO, "AccountManager.cpp: logoutRequest: Logout successful.");
        setJwt("");
        setLogin("");
        setPassword("");
        return "Logout successful";
    } else {
        logger.Log(LogLevel::ERR, "AccountManager.cpp: logoutRequest: Logout failed. Response: " + response);
        return "Logout failed: " + response;
    }
}

string AccountManager::updateRequest(string data) {
    // data contains "password:login:password:email"
    size_t passwordEnd = data.find(":");
    string password = data.substr(0, passwordEnd);
    
    size_t newLoginStart = passwordEnd + 1;
    size_t newLoginEnd = data.find(":", newLoginStart);
    string newLogin = data.substr(newLoginStart, newLoginEnd - newLoginStart);
    
    size_t newPasswordStart = newLoginEnd + 1;
    size_t newPasswordEnd = data.find(":", newPasswordStart);
    string newPassword = data.substr(newPasswordStart, newPasswordEnd - newPasswordStart);
    
    string newEmail = data.substr(newPasswordEnd + 1);
    
    logger.Log(LogLevel::INFO, "AccountManager.cpp: updateRequest: login: " + newLogin + ", password: " + newPassword + ", email: " + newEmail);

    // Проверяем текущий пароль
    if (password != getPassword()) {
        logger.Log(LogLevel::ERR, "AccountManager.cpp: updateRequest: Incorrect password.");
        return "Incorrect password";
    }

    string jsonData = "{\"login\":\"" + newLogin + "\",\"password\":\"" + newPassword + "\",\"email\":\"" + newEmail + "\"}";

    CURL* curl;
    CURLcode res;
    string response = "";

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (!curl) {
        logger.Log(LogLevel::ERR, "AccountManager.cpp: updateRequest: curl_easy_init() failed");
        return "Initialization failed";
    }

    // Формируем заголовок Authorization с токеном
    struct curl_slist* headers = NULL;
    string authHeader = "Authorization: Bearer " + getJwt();
    headers = curl_slist_append(headers, authHeader.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, SERVER_IP "/user/update");
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonData.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, account_manager_WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, account_manager_HeaderCallback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, this); // Передаем указатель на текущий объект   

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        logger.Log(LogLevel::ERR, "AccountManager.cpp: updateRequest: curl_easy_perform() failed: " + string(curl_easy_strerror(res)));
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        return "Request failed";
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);

    // Проверяем успешность обновления
    if (response.find("updated success") != string::npos) {
        logger.Log(LogLevel::INFO, "AccountManager.cpp: updateRequest: Update successful.");
        setLogin(newLogin);
        setPassword(newPassword);
        return "Update completed";
    } else {
        logger.Log(LogLevel::ERR, "AccountManager.cpp: updateRequest: Update failed. Response: " + response);
        return "Update failed: " + response;
    }
}





