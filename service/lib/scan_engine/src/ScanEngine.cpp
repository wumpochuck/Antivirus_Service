#include "../include/ScanEngine.h"

namespace fs = std::filesystem;

string ScanEngine::scanFile(const char* filePath) {
    logger.Log(LogLevel::INFO, "ScanEngine.cpp: scanFile: filePath: " + string(filePath));

    if (!loadSignatures("C:/Antivirus/logs/signatures.bin")) {
        return "Error: signatures not found[endl]Try later";
    }

    std::vector<uint8_t> fileBytes;
    if (!readFileBytes(filePath, fileBytes)) {
        return "Error: cannot open or read file for scanning[endl]Try later";
    }

    std::string fileExt = getFileExtension(filePath);  

    // Проверяем каждую сигнатуру
    for (const auto& entry : signatures) {
        if (!entry.file_type.empty() && toUpper(fileExt) != entry.file_type) {
            logger.Log(LogLevel::INFO, "Skipping signature " + entry.name + " due to file type mismatch (" + entry.file_type + " != " + toUpper(fileExt) + ")");
        }
        else if (matchSignatureInFile(fileBytes, entry)) {
            logger.Log(LogLevel::INFO, "Signature detected: " + entry.name);
            return string(filePath) + " : detected signature -> " + entry.name;
        }
    }

    string report = "No threats found";
    return report;
}

string ScanEngine::scanDirectory(const char* dirPath) {
    logger.Log(LogLevel::INFO, "ScanEngine.cpp: scanDirectory: dirPath: " + string(dirPath));

    std::vector<string> infectedReports; 
    size_t scannedFilesCount = 0;

    try {
        for (const auto& entry : fs::recursive_directory_iterator(dirPath)) {
            if (entry.is_regular_file()) {
                scannedFilesCount++;
                const auto& path = entry.path();
                logger.Log(LogLevel::INFO, "Scanning file: " + path.string());

                string result = scanFile(path.string().c_str());

                if (!result.empty() && result != "No threats found" &&
                    result.find("Error:") == string::npos &&
                    result.find("No viruses found") == string::npos) 
                {
                    infectedReports.push_back(result);
                    logger.Log(LogLevel::INFO, "Virus detected: " + result);
                }
            }
        }
    } catch (const std::exception& e) {
        logger.Log(LogLevel::ERR, string("Error scanning directory: ") + e.what());
        return string("Error scanning directory[endl]") + e.what();
    }

    // Формируем отчет
    if (infectedReports.empty()) {
        return "Scanned files: " + std::to_string(scannedFilesCount) + "[endl]" +
               "Viruses found: 0[endl]" +
               "No viruses found in the directory";
    } else {
        string report = "Scanned files: " + std::to_string(scannedFilesCount) + "[endl]" +
                        "Viruses found: " + std::to_string(infectedReports.size()) + "[endl]" +
                        "Infected files and threats:[endl]";
        for (const auto& inf : infectedReports) {
            report += inf + "[endl]";
        }
        return report;
    }
}

string ScanEngine::listDrives() {
    DWORD drives = GetLogicalDrives();
    string response = "";
    for (char letter = 'A'; letter <= 'Z'; ++letter) {
        if (drives & (1 << (letter - 'A'))) {
            std::string drivePath = std::string(1, letter) + ":\\";
            logger.Log(LogLevel::INFO, "ScanEngine.cpp: listDrives: Detected drive: " + drivePath);
            response += drivePath.c_str() + scanDirectory(drivePath.c_str());
        }
    }
    return response;
}

// Скан по расписанию
void ScanEngine::startScheduledScan() {
    std::thread([this]() {
        while (true) {
            // Получаем текущее время
            auto now = std::chrono::system_clock::now();
            std::time_t now_c = std::chrono::system_clock::to_time_t(now);
            std::tm local_tm = *std::localtime(&now_c);

            // Целевое время — 12:00
            local_tm.tm_hour = 12;
            local_tm.tm_min = 0;
            local_tm.tm_sec = 0;

            // Конвертируем обратно в time_point
            std::chrono::system_clock::time_point nextScanTime =
                std::chrono::system_clock::from_time_t(std::mktime(&local_tm));

            // Если уже позже 12:00 — переходим на следующий день
            if (nextScanTime <= now) {
                nextScanTime += std::chrono::hours(24);
            }

            // Сколько ждать до следующего запуска
            std::chrono::duration<double> waitDuration = nextScanTime - now;

            logger.Log(LogLevel::INFO, "Scheduled scan: next run in " + std::to_string((int)waitDuration.count()) + " seconds");

            // Ждём до нужного времени
            std::this_thread::sleep_for(waitDuration);

            // Сканируем все диски
            logger.Log(LogLevel::INFO, "Scheduled scan started at 12:00");
            listDrives();
        }
    }).detach();
}



bool ScanEngine::matchSignatureInFile(const std::vector<uint8_t>& fileBytes, const SignatureEntry& entry) {
    logger.Log(LogLevel::INFO, "matchSignatureInFile: Checking signature '" + entry.name + "'");
    logger.Log(LogLevel::INFO, "matchSignatureInFile: Original offsets: start=" + std::to_string(entry.offset_start) +
               ", end=" + std::to_string(entry.offset_end) + ", file size=" + std::to_string(fileBytes.size()));

    if (entry.offset_start >= fileBytes.size()) {
        logger.Log(LogLevel::INFO, "matchSignatureInFile: offset_start >= file size — signature cannot be in file");
        return false;
    }

    // Подгоняем offset_end под размер файла, если нужно

    size_t adjusted_offset_end;
    if (entry.offset_end < fileBytes.size()) {
        adjusted_offset_end = entry.offset_end;
    } else {
        adjusted_offset_end = fileBytes.size();
    }


    if (entry.offset_start > adjusted_offset_end) {
        logger.Log(LogLevel::ERR, "matchSignatureInFile: offset_start > adjusted offset_end");
        return false;
    }

    size_t rangeLength = adjusted_offset_end - entry.offset_start;
    const size_t sigLen = entry.signature.size();

    logger.Log(LogLevel::INFO, "matchSignatureInFile: Adjusted rangeLength = " + std::to_string(rangeLength) +
               ", signature length = " + std::to_string(sigLen));

    if (sigLen > rangeLength) {
        logger.Log(LogLevel::INFO, "matchSignatureInFile: Signature length greater than adjusted search range — no match possible");
        return false;
    }

    // Поиск точного вхождения сигнатуры в диапазоне [offset_start, adjusted_offset_end)
    for (size_t pos = entry.offset_start; pos <= adjusted_offset_end - sigLen; ++pos) {
        // logger.Log(LogLevel::INFO, "matchSignatureInFile: Checking position " + std::to_string(pos));
        bool match = true;
        for (size_t i = 0; i < sigLen; ++i) {
            if (fileBytes[pos + i] != entry.signature[i]) {
                // logger.Log(LogLevel::INFO, "matchSignatureInFile: Byte mismatch at offset " + std::to_string(pos + i) +
                //            " (file: " + std::to_string(fileBytes[pos + i]) +
                //            ", signature: " + std::to_string(entry.signature[i]) + ")");
                match = false;
                break;
            }
        }
        if (match) {
            logger.Log(LogLevel::INFO, "matchSignatureInFile: Signature matched at position " + std::to_string(pos));
            return true;
        }
    }

    logger.Log(LogLevel::INFO, "matchSignatureInFile: Signature not found in given adjusted range");
    return false;
}

std::string toUpper(const std::string& s) {
    std::string res = s;
    std::transform(res.begin(), res.end(), res.begin(), [](unsigned char c) { return std::toupper(c); });
    return res;
}

std::string getFileExtension(const std::string& filePath) {
    size_t dotPos = filePath.find_last_of('.');
    if (dotPos == std::string::npos || dotPos == filePath.length() - 1) {
        return "";
    }
    return filePath.substr(dotPos + 1);  // без точки
}

// Загружаем сигнатуры из бинаря
bool ScanEngine::loadSignatures(const std::string& path) {

    // Очищаем вектор перед сканированием (в случае если подгружен новый)
    signatures.clear();

    std::ifstream in(path, std::ios::binary);
    if (!in) {
        logger.Log(LogLevel::ERR, "ScanEngine.cpp: loadSignatures: Failed to open signatures file: " + path);
        return false;
    }

    auto read_u32 = [&](std::ifstream& in) -> uint32_t {
        uint32_t val;
        in.read(reinterpret_cast<char*>(&val), sizeof(val));
        return val;
    };

    auto read_string = [&](std::ifstream& in, size_t len) -> std::string {
        std::vector<char> buf(len);
        in.read(buf.data(), len);
        return std::string(buf.begin(), buf.end());
    };

    uint32_t magic = read_u32(in);
    uint32_t version = read_u32(in);
    uint32_t count = read_u32(in);

    logger.Log(LogLevel::INFO, "ScanEngine.cpp: loadSignatures: Magic: " + std::to_string(magic));
    logger.Log(LogLevel::INFO, "ScanEngine.cpp: loadSignatures: Version: " + std::to_string(version));
    logger.Log(LogLevel::INFO, "ScanEngine.cpp: loadSignatures: Entries: " + std::to_string(count));

    signatures.clear();

    for (uint32_t i = 0; i < count; ++i) {
        SignatureEntry entry;

        in.read(reinterpret_cast<char*>(entry.id.data()), 16);

        uint32_t name_len = read_u32(in);
        entry.name = read_string(in, name_len);

        in.read(reinterpret_cast<char*>(entry.signature.data()), 8);

        uint32_t hash_len = read_u32(in);
        entry.hash.resize(hash_len);
        in.read(reinterpret_cast<char*>(entry.hash.data()), hash_len);

        entry.remainder_len = read_u32(in);

        uint32_t type_len = read_u32(in);
        entry.file_type = read_string(in, type_len);

        entry.offset_start = read_u32(in);
        entry.offset_end = read_u32(in);

        signatures.push_back(std::move(entry));

        if (in.eof()) {
            logger.Log(LogLevel::ERR, "ScanEngine.cpp: loadSignatures: Unexpected EOF at entry " + std::to_string(i));
            return false;
        }
    }

    logger.Log(LogLevel::INFO, "ScanEngine.cpp: loadSignatures: Loaded " + std::to_string(signatures.size()) + " signatures.");
    return true;
}

// Считываем файл в виде байтов
bool ScanEngine::readFileBytes(const std::string& filePath, std::vector<uint8_t>& outBytes) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        logger.Log(LogLevel::ERR, "ScanEngine.cpp: readFileBytes: Cannot open file: " + filePath);
        return false;
    }

    file.seekg(0, std::ios::end);
    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    if (fileSize <= 0) {
        logger.Log(LogLevel::ERR, "ScanEngine.cpp: readFileBytes: Empty or invalid file: " + filePath);
        return false;
    }

    outBytes.resize(static_cast<size_t>(fileSize));
    if (!file.read(reinterpret_cast<char*>(outBytes.data()), fileSize)) {
        logger.Log(LogLevel::ERR, "ScanEngine.cpp: readFileBytes: Error reading file: " + filePath);
        return false;
    }

    logger.Log(LogLevel::INFO, "ScanEngine.cpp: readFileBytes: File size: " + std::to_string(fileSize));
    return true;
}

// Логирование
void ScanEngine::formatSignaturesReport() {
    logger.Log(LogLevel::INFO, "ScanEngine.cpp: formatSignaturesReport: Formatting signatures report...");

    for (const auto& entry : signatures) {
        string sigReport = "";
        sigReport += "ID: " + std::to_string(entry.id[0]) + ",";
        sigReport += " Name: " + entry.name + ",";
        sigReport += " Signature: ";
        for (const auto& byte : entry.signature) {
            sigReport += std::to_string(byte) + " ";
        } 
        sigReport += " Hash: ";
        for (const auto& byte : entry.hash) {
            sigReport += std::to_string(byte) + " ";
        }
        sigReport += " Remainder Length: " + std::to_string(entry.remainder_len) + ",";
        sigReport += " File Type: " + entry.file_type + ",";
        sigReport += " Offset Start: " + std::to_string(entry.offset_start) + ",";
        sigReport += " Offset End: " + std::to_string(entry.offset_end) + "";
        logger.Log(LogLevel::INFO, "ScanEngine.cpp: formatSignaturesReport: " + sigReport);
    }
}




