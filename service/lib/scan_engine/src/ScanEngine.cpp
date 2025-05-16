#include "../include/ScanEngine.h"


string ScanEngine::scanFile(const char* filePath) {

    logger.Log(LogLevel::INFO, "ScanEngine.cpp: scanFile: filePath: " + string(filePath));

    // Здесь будет код для сканирования файла
    // Возвращается текст строкой по типу:

    /*
    
    Обнаружено угроз : 0 \n
    Проверено файлов : 1 \n

    */

    if(1){ // Заглушка
        return "No viruses found in the file";
    }

    return "";
}

string ScanEngine::scanDirectory(const char* dirPath) {
    
    logger.Log(LogLevel::INFO, "ScanEngine.cpp: scanDirectory: dirPath: " + string(dirPath));
    
    // Здесь будет код для сканирования директории

    if(1){ // Заглушка
        return "No viruses found in the directory";
    }

    return "";
}