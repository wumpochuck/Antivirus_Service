#pragma once

#ifndef LICENSE_SERVICE_H
#define LICENSE_SERVICE_H

#include <string>

#include "../../../include/Config.h"
#include "../../logger/include/Logger.h"
#include "../../curl-8.11.0/include/curl/curl.h"
#include "../../account_manager/include/account_manager.h"

using namespace std;

class LicenseService {
    private:
        string license_code;
        string ticket;

        static size_t license_service_WriteCallback(void* contents, size_t size, size_t nmemb, std::string* response);
        static size_t license_service_HeaderCallback(char* buffer, size_t size, size_t nitems, void* userdata);
    public:
        LicenseService() : license_code(""), ticket("") {}
        LicenseService(string license_code, string ticket) : license_code(license_code), ticket(ticket) {}

        string getLicenseCode() const { return license_code; }
        string getTicket() const { return ticket; }

        void setLicenseCode(string newLicenseCode) { license_code = newLicenseCode; }
        void setTicket(string newTicket) { ticket = newTicket; }

        string activateLicenseRequest(string data);
        string licenseInfoRequest(string data);
        string licenseUpdateRequest(string data);

};

extern LicenseService licenseService;

#endif  // LICENSE_SERVICE_H