#pragma once
#include "resource.h"

class Logger {
public:
    static void addToLog(const char* message, const char* userId) {
        std::ofstream logFile("log.txt", std::ios::app);
        if (logFile.is_open()) {
            time_t rawtime = time(NULL);
            tm* ptm = localtime(&rawtime);
            char* currentTime = (char*)malloc(sizeof(char) * 100);
            strftime(currentTime, 100, "%d/%m/%y\t%H:%M:%S", ptm);
            logFile << currentTime << "\t" << message << "\t" << userId << std::endl;
            logFile.close();
            free(currentTime);
        }
    }
};