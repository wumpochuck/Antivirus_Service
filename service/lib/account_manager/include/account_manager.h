#pragma once

#ifndef ACCOUNT_MANAGER_H
#define ACCOUNT_MANAGER_H

#include <string>

#include "../../../include/Config.h"
#include "../../logger/include/Logger.h"
#include "../../curl-8.11.0/include/curl/curl.h"

using namespace std;

class AccountManager {
    private:
        string jwt;
        string login;
        string password;

        static size_t account_manager_WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response);
        static size_t account_manager_HeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata);
    public:
        AccountManager() : jwt(""), login("") {}
        AccountManager(string jwt, string login) : jwt(jwt), login(login) {}

        string getJwt() const { return jwt; }
        string getLogin() const { return login; }
        string getPassword() const { return password; }

        void setJwt(string newJwt) { jwt = newJwt; }
        void setLogin(string newLogin) { login = newLogin; }
        void setPassword(string newPassword) { password = newPassword; }

        bool isLoggedIn() const { return !jwt.empty(); }
        void logout() { jwt.clear(); login.clear(); }

        string loginRequest(string data);
        string registerRequest(string data);
        string logoutRequest();
        string updateRequest(string data);

};

extern AccountManager accountManager;

#endif  // ACCOUNT_MANAGER_H