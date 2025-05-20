#ifndef SCAN_ENGINE_H
#define SCAN_ENGINE_H

#include <string>
#include <fstream>
#include <vector>
#include <array>
#include <iomanip>
#include <sstream>

#include "../../logger/include/Logger.h"

using namespace std;

struct SignatureEntry {
    array<uint8_t, 16> id;
    string name;
    array<uint8_t, 8> signature;
    vector<uint8_t> hash;
    uint32_t remainder_len;
    string file_type;
    uint32_t offset_start;
    uint32_t offset_end;
};

class ScanEngine {
private:
    vector<SignatureEntry> signatures;

    bool loadSignatures(const string& path);
    void formatSignaturesReport();
    bool readFileBytes(const std::string& filePath, std::vector<uint8_t>& buffer);

    // Для поиска сигнатуры в промежутке
    bool matchSignatureInFile(const std::vector<uint8_t>& fileBytes,const SignatureEntry& entry);

    bool scanFileWithSignatures(const std::string& filePath,const std::vector<SignatureEntry>& signatures,std::vector<std::string>& detectedThreats);
    

public:
    string scanFile(const char* filePath);
    string scanDirectory(const char* dirPath);
    
};

extern ScanEngine scanEngine;

#endif // SCAN_ENGINE_H