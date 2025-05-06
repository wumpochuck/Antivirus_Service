#include "../include/logger.h"

int main() {
    Logger logger("logfile.txt");
    logger.Log(LogLevel::INFO, "This is an info message.");
    logger.Log(LogLevel::ERR, "This is an error message.");
    return 0;
}