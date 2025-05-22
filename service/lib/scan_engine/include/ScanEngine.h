#ifndef SCAN_ENGINE_H
#define SCAN_ENGINE_H

#include <string>
#include <fstream>
#include <vector>
#include <array>
#include <iomanip>
#include <sstream>

#include <algorithm> 
#include <filesystem>
#include <windows.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <ctime>

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

string toUpper(const std::string& s);
string getFileExtension(const std::string& filePath) ;

class ScanEngine {
private:
    vector<SignatureEntry> signatures;

    bool loadSignatures(const string& path);
    void formatSignaturesReport();
    bool readFileBytes(const std::string& filePath, std::vector<uint8_t>& buffer);
    

public:
    string scanFile(const char* filePath);
    string scanDirectory(const char* dirPath);
    void startScheduledScan();
    
    bool matchSignatureInFile(const std::vector<uint8_t>& fileBytes, const SignatureEntry& entry);
    string listDrives();
    
};

extern ScanEngine scanEngine;

#endif // SCAN_ENGINE_H