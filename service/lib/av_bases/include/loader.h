#ifndef AV_BASES_HTTPCLIENT_H
#define AV_BASES_HTTPCLIENT_H

#include <iostream>
#include <curl/curl.h>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include <filesystem>

#include <string>
#include <fstream>
#include <unordered_map>
#include <filesystem>
#include <iterator>     // для std::istreambuf_iterator


#include "../../logger/include/Logger.h"

class ServerLoader {
public:
    ServerLoader();
    ~ServerLoader();

    void performGetRequest(const std::string& url, const std::string& token);

private:
    static size_t WriteToStringCallback(void* contents, size_t size, size_t nmemb, std::string* output);
    static size_t WriteHeadersCallback(void* contents, size_t size, size_t nmemb, std::string* headers);
    std::string extractBoundary(const std::string& headers);
    void parseAndSaveMultipartResponse(const std::string& response, const std::string& boundary);
};

extern ServerLoader serverLoader;

#endif // AV_BASES_HTTPCLIENT_H