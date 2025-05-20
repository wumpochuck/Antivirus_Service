#include "../include/ScanEngine.h"

string ScanEngine::scanFile(const char* filePath) {
    logger.Log(LogLevel::INFO, "ScanEngine.cpp: scanFile: filePath: " + string(filePath));

    if (!loadSignatures("C:/Antivirus/logs/signatures.bin")) {
        return "Error: signatures not found[endl]Try later";
    }

    std::vector<uint8_t> fileBytes;
    if (!readFileBytes(filePath, fileBytes)) {
        return "Error: cannot open or read file for scanning[endl]Try later";
    }

    // log signatures
    formatSignaturesReport();


    // log file bytes
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (const auto& byte : fileBytes) {
        oss << std::setw(2) << static_cast<int>(byte) << " ";
    }
    logger.Log(LogLevel::INFO, "ScanEngine.cpp: scanFile: File bytes: " + oss.str());
    logger.Log(LogLevel::INFO, "ScanEngine.cpp: scanFile: File size: " + std::to_string(fileBytes.size()) + " bytes");
    

    string report = "pizda";
    return report;
}




string ScanEngine::scanDirectory(const char* dirPath) {
    
    logger.Log(LogLevel::INFO, "ScanEngine.cpp: scanDirectory: dirPath: " + string(dirPath));
    
    // Здесь будет код для сканирования директории

    if(1){ // Заглушка
        return "No viruses found in the directory";
    }

    return "";
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

    file.unsetf(std::ios::skipws);
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    outBytes.reserve(fileSize);
    outBytes.insert(outBytes.begin(),
                    std::istream_iterator<uint8_t>(file),
                    std::istream_iterator<uint8_t>());

    logger.Log(LogLevel::INFO, "ScanEngine.cpp: readFileBytes: File size: " + std::to_string(fileSize));
    return true;
}


// Для поиска сигнатуры в промежутке
bool ScanEngine::matchSignatureInFile(const std::vector<uint8_t>& fileBytes, const SignatureEntry& entry) {
    size_t fileSize = fileBytes.size();
    size_t sigSize = entry.signature.size();

    if (fileSize < entry.offset_start + sigSize) {
        logger.Log(LogLevel::INFO, "ScanEngine.cpp: matchSignatureInFile: File too small for signature " + entry.name);
        return false; // Файл слишком маленький
    }

    uint32_t start = std::min(entry.offset_start, static_cast<uint32_t>(fileSize));
    uint32_t end = std::min(entry.offset_end, static_cast<uint32_t>(fileSize - sigSize));

    const uint8_t firstByte = entry.signature[0];

    uint32_t pos = start;
    while (pos <= end) {
        // Ищем первый байт сигнатуры
        if (fileBytes[pos] == firstByte) {
            // Проверяем остальные байты
            bool match = true;
            for (size_t i = 1; i < sigSize; ++i) {
                if (fileBytes[pos + i] != entry.signature[i]) {
                    match = false;

                    // Логируем несовпадение в hex формате
                    std::ostringstream oss;
                    oss << std::hex << std::setfill('0');
                    oss << "Mismatch at pos " << std::setw(2) << pos + i
                        << ": fileByte=0x" << std::setw(2) << static_cast<int>(fileBytes[pos + i])
                        << ", sigByte=0x" << std::setw(2) << static_cast<int>(entry.signature[i])
                        << " for sig " << entry.name;
                    logger.Log(LogLevel::INFO, oss.str());

                    break;
                }
            }
            if (match) {
                logger.Log(LogLevel::INFO, "ScanEngine.cpp: matchSignatureInFile: Found match for signature " + entry.name + " at pos " + std::to_string(pos));
                return true;
            } else {
                pos++;  // если не совпало, сдвигаемся на следующий байт и ищем снова первый байт
            }
        } else {
            pos++;  // если первый байт не совпал, сдвигаемся вперед
        }
    }

    return false;
}


// bool ScanEngine::scanFileWithSignatures(const std::string& filePath,const std::vector<SignatureEntry>& signatures,std::vector<std::string>& detectedThreats) {
//     logger.Log(LogLevel::INFO, "ScanEngine.cpp: scanFileWithSignatures: Scanning " + filePath);

//     std::ifstream file(filePath, std::ios::binary);
//     if (!file) {
//         logger.Log(LogLevel::ERR, "ScanEngine.cpp: scanFileWithSignatures: Failed to open file: " + filePath);
//         return false;
//     }

//     // Читаем весь файл в память
//     std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
//     logger.Log(LogLevel::INFO, "ScanEngine.cpp: scanFileWithSignatures: File size: " + std::to_string(buffer.size()) + " bytes");

//     for (const auto& entry : signatures) {
//         if (matchSignatureInFile(buffer, entry)) {
//             detectedThreats.push_back(entry.name);
//         }
//     }

//     return true;
// }











