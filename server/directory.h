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

    static void createDirectory(Message& msg) {
        char path[256];
        snprintf(path, sizeof(path), "%s/%s", SERVER_FOLDER, msg.payload);
        if (mkdir(path, 0777) == 0) {
            msg.opcode = CREATE_FOLDER_SUCCESS;
        } else {
            msg.opcode = FOLDER_ERROR;
        }
    }

    static void deleteDirectory(Message& msg) {
        char path[256];
        snprintf(path, sizeof(path), "%s/%s", SERVER_FOLDER, msg.payload);
        if (rmdir(path) == 0) {
            msg.opcode = CREATE_FOLDER_SUCCESS;
        } else {
            msg.opcode = FOLDER_ERROR;
        }
    }

    static void renameDirectory(Message& msg) {
        char oldName[128], newName[128];
        sscanf(msg.payload, "%s %s", oldName, newName);

        char oldPath[256], newPath[256];
        snprintf(oldPath, sizeof(oldPath), "%s/%s", SERVER_FOLDER, oldName);
        snprintf(newPath, sizeof(newPath), "%s/%s", SERVER_FOLDER, newName);

        if (rename(oldPath, newPath) == 0) {
            msg.opcode = CREATE_FOLDER_SUCCESS;
        } else {
            msg.opcode = FOLDER_ERROR;
        }
    }
};