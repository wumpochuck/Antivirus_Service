#pragma once;

#include <string.h>
#include <map>
#include <fstream>
#include <format>
#include <winbase.h>

#define LOGFILE "D:\Desktop\Antivirus\Antivirus_Service\log\service.log";

enum LOGTYPE{
    INFO,
    WARNING,
    ERROR,
};
