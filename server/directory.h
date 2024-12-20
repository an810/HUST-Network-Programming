#pragma once
#include "resource.h"

class Directory {
public:
    static void listFiles(ClientInfo& client, Message& msg) {

        DIR* dir = opendir(client.currentDir);
        if (!dir) return;

        struct dirent* entry;
        std::string result;

        result += "Current directory: ";
        result += client.currentDir;
        result += "\n";

        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] != '.') {
                result += entry->d_name;
                result += "\n";
            }
        }

        strcpy(msg.payload, result.c_str());
        closedir(dir);
    }

    static bool changeDirectory(ClientInfo& client, Message& msg) {
        std::string newDir = msg.payload;
        char newPath[256];

        // Check if the current directory is SERVER_FOLDER
        if (strcmp(client.currentDir, SERVER_FOLDER) == 0 && strcmp(newDir.c_str(), "..") == 0) {
            std::cout << "Error: Cannot go above the SERVER_FOLDER directory." << std::endl;
            msg.opcode = FOLDER_NOT_FOUND;
            strcpy(msg.payload, client.currentDir);
            return false;
        }

        if (strcmp(newDir.c_str(), "..") == 0) {
            // Handle the ".." case: move one level up
            char* lastSlash = strrchr(client.currentDir, '/');
            if (lastSlash && lastSlash != client.currentDir) {
                // Truncate the last directory from the path
                *lastSlash = '\0';
            } 
        } else {
            // Normal directory change
            snprintf(newPath, sizeof(newPath), "%s/%s", client.currentDir, newDir.c_str());
            if (opendir(newPath)) {
                strcpy(client.currentDir, newPath);
                std::cout << "Change directory - Current directory: " << client.currentDir << std::endl;
                msg.opcode = CHANGE_SUCCESS;
                strcpy(msg.payload, client.currentDir);
                return true;
            }
            std::cout << "Directory not found: " << newPath << std::endl;
            msg.opcode = FOLDER_NOT_FOUND;
            strcpy(msg.payload, client.currentDir);
            return false;
        }

        std::cout << "Changed directory to " << client.currentDir << std::endl;
        msg.opcode = CHANGE_SUCCESS;
        strcpy(msg.payload, client.currentDir);
        return true;
    }

    static void createDirectory(ClientInfo& client, Message& msg) {
        char path[256];
        snprintf(path, sizeof(path), "%s/%s", client.currentDir, msg.payload);
        if (mkdir(path, 0777) == 0) {
            msg.opcode = CREATE_FOLDER_SUCCESS;
        } else {
            msg.opcode = FOLDER_ALREADY_EXIST;
        }
    }

    static void deleteDirectory(ClientInfo& client, Message& msg) {
        char path[256];
        snprintf(path, sizeof(path), "%s/%s", client.currentDir, msg.payload);

        DIR* dir = opendir(path);
        if (!dir) {
            std::cout << "Directory not found: " << path << std::endl;
            msg.opcode = FOLDER_NOT_FOUND;
            send(client.socket, &msg, sizeof(msg), 0);
            return;
        }

        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            // Skip "." and ".." entries
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            std::string entryPath = std::string(path) + "/" + entry->d_name;

            struct stat entryStat;
            if (stat(entryPath.c_str(), &entryStat) == 0) {
                if (S_ISDIR(entryStat.st_mode)) {
                    // If it's a directory, recursively delete its contents
                    Message subMsg;
                    strcpy(subMsg.payload, entry->d_name);
                    deleteDirectory(client, subMsg); // Recursive call to delete subdirectory
                } else {
                    // If it's a file, delete it
                    if (remove(entryPath.c_str()) == 0) {
                        std::cout << "Deleted file: " << entryPath << std::endl;
                    } else {
                        std::cout << "Failed to delete file: " << entryPath << std::endl;
                    }
                }
            }
        }

        closedir(dir);

        // After deleting all files and subdirectories, remove the directory itself
        if (rmdir(path) == 0) {
            std::cout << "Deleted folder: " << path << std::endl;
            msg.opcode = DELETE_FOLDER_SUCCESS;
        } else {
            std::cout << "Failed to delete folder: " << path << std::endl;
            msg.opcode = FOLDER_NOT_FOUND;
        }

        // // Send the response back to the client
        // send(client.socket, &msg, sizeof(msg), 0);
    }

    static void renameDirectory(ClientInfo& client, Message& msg) {
        char oldName[128], newName[128];
        sscanf(msg.payload, "%s %s", oldName, newName);

        char oldPath[256], newPath[256];
        snprintf(oldPath, sizeof(oldPath), "%s/%s", client.currentDir, oldName);
        snprintf(newPath, sizeof(newPath), "%s/%s", client.currentDir, newName);

        if (rename(oldPath, newPath) == 0) {
            msg.opcode = CREATE_FOLDER_SUCCESS;
        } else {
            msg.opcode = FOLDER_NOT_FOUND;
        }
    }
};