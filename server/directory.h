#pragma once
#include "resource.h"

class Directory {
public:
    static void listFiles(const char* path, char* output) {
        DIR* dir = opendir(path);
        if (!dir) return;
        
        struct dirent* entry;
        std::string result;
        
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] != '.') {
                result += entry->d_name;
                result += "\n";
            }
        }
        
        strcpy(output, result.c_str());
        closedir(dir);
    }

    static bool changeDirectory(ClientInfo& client, const char* newDir) {
        char newPath[256];
        snprintf(newPath, sizeof(newPath), "%s/%s", client.currentDir, newDir);
        DIR* dir = opendir(newPath);
        if (dir) {
            strcpy(client.currentDir, newPath);
            closedir(dir);
            return true;
        }
        return false;
    }
};