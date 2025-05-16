#ifndef SCAN_ENGINE_H
#define SCAN_ENGINE_H

#include <string>

#include "../../logger/include/Logger.h"

using namespace std;

class ScanEngine {
public:
    string scanFile(const char* filePath); 
    string scanDirectory(const char* dirPath);

};

extern ScanEngine scanEngine;

#endif // SCAN_ENGINE_H