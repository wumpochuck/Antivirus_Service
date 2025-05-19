#include "../include/loader.h"


// -------------- SERVER LOADER -------------- //

ServerLoader::ServerLoader() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

ServerLoader::~ServerLoader() {
    curl_global_cleanup();
}

size_t ServerLoader::WriteToStringCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

size_t ServerLoader::WriteHeadersCallback(void* contents, size_t size, size_t nmemb, std::string* headers) {
    size_t totalSize = size * nmemb;
    headers->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

std::string ServerLoader::extractBoundary(const std::string& headers) {
    std::regex boundaryRegex(R"(boundary=([^\s;]+))");
    std::smatch match;
    if (std::regex_search(headers, match, boundaryRegex)) {
        return "--" + match[1].str(); // Add "--" to match the boundary format
    }
    return "";
}

/*
void ServerLoader::parseAndSaveMultipartResponse(const std::string& response, const std::string& boundary) {
    size_t pos = 0;
    size_t boundaryLength = boundary.length();
    int fileIndex = 0;

    while ((pos = response.find(boundary, pos)) != std::string::npos) {
        size_t start = pos + boundaryLength + 2; // Skip boundary and CRLF
        size_t end = response.find(boundary, start);

        if (end == std::string::npos) {
            break; // No more parts
        }

        // Extract the part
        std::string part = response.substr(start, end - start);

        // Find the header-body separator (double CRLF)
        size_t headerEnd = part.find("\r\n\r\n");
        if (headerEnd == std::string::npos) {
            pos = end;
            continue; // Skip invalid part
        }

        // Extract headers and body
        std::string headers = part.substr(0, headerEnd);
        std::string body = part.substr(headerEnd + 4); // Skip the double CRLF

        // 1) ybrat cycle
        // 2) esli manifesta net, to prodoljit (sozdanie i td)
        // 3) esle est, snachala proverki po hasham
        // 4) esli oni ne sovpali, to zamenit manifest i signatures
        // 5) inache return




        // Determine the filename from headers (if available)
        std::string filename = "file_" + std::to_string(fileIndex++) + ".bin";
        size_t dispositionPos = headers.find("Content-Disposition:");
        if (dispositionPos != std::string::npos) {
            size_t filenamePos = headers.find("filename=", dispositionPos);
            if (filenamePos != std::string::npos) {
                size_t startQuote = headers.find('"', filenamePos);
                size_t endQuote = headers.find('"', startQuote + 1);
                if (startQuote != std::string::npos && endQuote != std::string::npos) {
                    filename = headers.substr(startQuote + 1, endQuote - startQuote - 1);
                }
            }
        }

        // Save the body to a file C:\Antivirus\logs + filename
        std::ofstream outFile("C:/Antivirus/logs/" + filename, std::ios::binary);
        if (outFile.is_open()) {
            outFile.write(body.c_str(), body.size());
            outFile.close();
            // full path
            logger.Log(LogLevel::INFO, "Saved file: " + std::filesystem::absolute(filename).string());
        } else {
            logger.Log(LogLevel::ERR, "Failed to save file: " + filename);
        }

        pos = end;
    }
}
*/

void ServerLoader::performGetRequest(const std::string& url, const std::string& token) {
    CURL* curl;
    CURLcode res;
    std::string response;
    std::string headers;

    curl = curl_easy_init();
    if (curl) {
        // Set URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // Set Authorization header with token
        struct curl_slist* headerList = nullptr;
        std::string authHeader = "Authorization: Bearer " + token;
        headerList = curl_slist_append(headerList, authHeader.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);

        // Set callbacks for writing data and headers
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteToStringCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, WriteHeadersCallback);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headers);

        // Perform the request
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            logger.Log(LogLevel::ERR, "GET request failed: " + std::string(curl_easy_strerror(res)));
        } else {
            logger.Log(LogLevel::INFO, "GET request successful.");

            // logger.Log(LogLevel::INFO, "Response: " + response);

            // Extract boundary from headers
            std::string boundary = extractBoundary(headers);
            logger.Log(LogLevel::INFO, "Boundary: " + boundary);
            if (boundary.empty()) {
                logger.Log(LogLevel::ERR, "Failed to extract boundary from headers.");
            } else {
                parseAndSaveMultipartResponse(response, boundary);
            }
        }

        // Clean up resources
        curl_slist_free_all(headerList);
        curl_easy_cleanup(curl);
    }
}

void ServerLoader::parseAndSaveMultipartResponse(const std::string& response, const std::string& boundary) {
    const std::string logsPath = "C:/Antivirus/logs/";
    const std::string manifestPath = logsPath + "manifest.bin";
    const std::string signaturesPath = logsPath + "signatures.bin";

    std::unordered_map<std::string, std::string> parts;

    size_t pos = 0;
    size_t boundaryLength = boundary.length();

    // Ищем первую часть (manifest)
    for (int partIndex = 0; partIndex < 2; ++partIndex) {
        pos = response.find(boundary, pos);
        if (pos == std::string::npos) break;

        size_t start = pos + boundaryLength + 2;
        size_t end = response.find(boundary, start);
        if (end == std::string::npos) break;

        std::string part = response.substr(start, end - start);
        size_t headerEnd = part.find("\r\n\r\n");
        if (headerEnd == std::string::npos) {
            pos = end;
            continue;
        }

        std::string headers = part.substr(0, headerEnd);
        std::string body = part.substr(headerEnd + 4);

        std::string filename = "file_" + std::to_string(partIndex) + ".bin";
        size_t dispositionPos = headers.find("Content-Disposition:");
        if (dispositionPos != std::string::npos) {
            size_t filenamePos = headers.find("filename=", dispositionPos);
            if (filenamePos != std::string::npos) {
                size_t startQuote = headers.find('"', filenamePos);
                size_t endQuote = headers.find('"', startQuote + 1);
                if (startQuote != std::string::npos && endQuote != std::string::npos) {
                    filename = headers.substr(startQuote + 1, endQuote - startQuote - 1);
                }
            }
        }

        parts[filename] = body;
        pos = end;
    }

    // Проверка наличия manifest.bin
    if (!std::filesystem::exists(manifestPath)) {
        logger.Log(LogLevel::INFO, "manifest.bin not found, saving all files.");

        for (const auto& [filename, data] : parts) {
            std::string fullPath = logsPath + filename;
            std::ofstream out(fullPath, std::ios::binary);
            if (out.is_open()) {
                out.write(data.data(), data.size());
                out.close();
                logger.Log(LogLevel::INFO, "Saved file: " + fullPath);
            } else {
                logger.Log(LogLevel::ERR, "Failed to save file: " + fullPath);
            }
        }
        return;
    }

    // Сравниваем текущий manifest.bin с новым
    std::ifstream currentManifest(manifestPath, std::ios::binary);
    std::string currentData((std::istreambuf_iterator<char>(currentManifest)),
                            std::istreambuf_iterator<char>());
    currentManifest.close();

    auto it = parts.find("manifest.bin");
    if (it == parts.end()) {
        logger.Log(LogLevel::ERR, "manifest.bin not found in response.");
        return;
    }

    const std::string& newManifestData = it->second;

    if (currentData == newManifestData) {
        logger.Log(LogLevel::INFO, "Manifest matches existing file. Skipping update.");
        return;
    }

    // Обновляем manifest и signatures
    logger.Log(LogLevel::INFO, "Manifest differs. Updating files...");

    for (const auto& [filename, data] : parts) {
        std::string fullPath = logsPath + filename;
        std::ofstream out(fullPath, std::ios::binary);
        if (out.is_open()) {
            out.write(data.data(), data.size());
            out.close();
            logger.Log(LogLevel::INFO, "Updated file: " + fullPath);
        } else {
            logger.Log(LogLevel::ERR, "Failed to write file: " + fullPath);
        }
    }
}



