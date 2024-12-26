#pragma once
#include "resource.h"

class FileHandler {
public:
    static void addToLog(const char* message, const char* userId) {
        std::ofstream logFile("log.txt", std::ios::app);
        if (logFile.is_open()) {
            time_t rawtime = time(NULL);
	        tm* ptm = localtime(&rawtime);
            char* currentTime = (char*)malloc(sizeof(char) * 100);;
            strftime(currentTime, 100, "%d/%m/%y\t%H:%M:%S", ptm);
            logFile << currentTime << "\t" << message << "\t" << userId << std::endl;
            logFile.close();
            free(currentTime);
        }
    }
    
    static void handleDelete(ClientInfo& client, Message& msg) {
        std::cout << "Delete file - Client current dir: " << client.currentDir << std::endl;
        std::string filepath = std::string(client.currentDir) + "/" + msg.payload;
        std::cout << "Delete file - Filepath: " << filepath << std::endl;
        if (remove(filepath.c_str()) == 0) {
            msg.opcode = DELETE_FILE_SUCCESS;
            std::string message = "Delete file " + filepath;
            addToLog(message.c_str(), client.userId);
            std::cout << "Delete file - File deleted successfully " << msg.opcode << std::endl;
        } else {
            msg.opcode = FILE_NOT_FOUND;
            std::cout << "Delete file - File not found " << msg.opcode << std::endl;
        }

    }

    static void searchInSubfolders(const std::string& basePath, const std::string& fileName, std::vector<std::string>& foundPaths) {
        DIR* dir = opendir(basePath.c_str());
        if (!dir) return;

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

            std::string path = basePath + "/" + entry->d_name;
            if (isDirectory(path.c_str())) {
                searchInSubfolders(path, fileName, foundPaths);
            } else if (strcmp(entry->d_name, fileName.c_str()) == 0) {
                foundPaths.push_back(path);
            }
        }
        closedir(dir);
    }

    static void handleSearchFile(ClientInfo& client, Message& msg) {
        std::vector<std::string> foundPaths;
        searchInSubfolders(SERVER_FOLDER, msg.payload, foundPaths);

        if (!foundPaths.empty()) {
            std::string allPaths;
            for (const auto& path : foundPaths) {
                allPaths += path + "\n"; // Combine paths with newline separator
            }

            strncpy(msg.payload, allPaths.c_str(), sizeof(msg.payload) - 1);
            msg.payload[sizeof(msg.payload) - 1] = '\0'; // Ensure null-termination
            msg.opcode = SEARCH_FILE_SUCCESS;
        } else {
            msg.opcode = FILE_NOT_FOUND;
        }
    }


    static void handleUpload(ClientInfo& client, Message& msg) {
        std::string filepath = std::string(client.currentDir) + "/" + msg.payload;
        client.file = fopen(filepath.c_str(), "wb");
        client.bytesLeft = msg.length;
    }

    static void handleDataUpload(ClientInfo& client, Message& msg) {
        if (client.file) {
            fwrite(msg.payload, 1, msg.length, client.file);
            client.bytesLeft -= msg.length;

            if (client.bytesLeft == 0) {
                fclose(client.file);
                client.file = nullptr;
                // // Send upload success message
                // send(client.socket, &msg, sizeof(msg), 0);
                // msg.opcode = UPLOAD_SUCCESS;
                std::string message = "Upload file " + std::string(client.filename);
                addToLog(message.c_str(), client.userId);
            }
        }
    }

    // static void handleDownload(ClientInfo& client, Message& msg) {
    //     std::string filepath = std::string(client.currentDir) + "/" + msg.payload;
    //     client.file = fopen(filepath.c_str(), "rb");
    //     if (client.file) {
    //         fseek(client.file, 0, SEEK_END);
    //         client.fileSize = ftell(client.file);
    //         client.bytesLeft = client.fileSize;
    //         rewind(client.file);
    //         strcpy(client.filename, msg.payload);

    //         msg.opcode = DATA_DOWN;
    //         handleDataDown(client, msg);
    //     } else {
    //         msg.opcode = FILE_NOT_FOUND;
    //     }
    // }

    static bool isDirectory(const char* path) {
        struct stat st;
        // string currentPath = string(SERVER_FOLDER) + "/" + path;
        if (stat(path, &st) == 0) {
            return S_ISDIR(st.st_mode);
        }
        return false;
    }

    // This function will be responsible for handling the file download
    static void handleFileDownload(ClientInfo& client, const std::string& filePath, Message& msg) {
        std::cout << "Handle File Download - File Path: " << filePath << std::endl;
        client.file = fopen(filePath.c_str(), "rb");
        if (client.file) {
            fseek(client.file, 0, SEEK_END);
            client.fileSize = ftell(client.file);
            client.bytesLeft = client.fileSize;
            rewind(client.file);
            strcpy(client.filename, msg.payload);
            
            std::cout << "Handle File Download - PayLoad: " << msg.payload << std::endl;

            msg.opcode = DATA_DOWN;
            handleDataDown(client, msg);
        } else {
            msg.opcode = FILE_NOT_FOUND;  
        }
    }

    static void handleFolderDownload(ClientInfo& client, const std::string& folderPath, Message& msg) {
        DIR* dir = opendir(folderPath.c_str());
        if (!dir) {
            msg.opcode = FOLDER_NOT_FOUND;
            // send(client.socket, &msg, sizeof(msg), 0);  // Folder doesn't exist
            return;
        }

        msg.opcode = CREATE_FOLDER;
        send(client.socket, &msg, sizeof(msg), 0);  // Notify client to create the folder

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;  // Skip "." and ".."

            std::string itemPath = folderPath + "/" + entry->d_name;
            strcpy(msg.payload, entry->d_name);

            if (isDirectory(itemPath.c_str())) {
                // If it's a subfolder, recursively handle it
                // strcpy(msg.payload, entry->d_name);
                // send(client.socket, &msg, sizeof(msg), 0);
                std::cout << "Handle Folder Download - Subfolder: " << itemPath << std::endl;
                handleFolderDownload(client, itemPath, msg);  // Recursively handle subfolders
            } else {
                // If it's a file, handle it as a file
                // strcpy(msg.payload, entry->d_name);
                std::cout << "Handle Folder Download - File: " << itemPath << std::endl;
                // send filename to client
                msg.opcode = DATA_DOWN;
                send(client.socket, &msg, sizeof(msg), 0);
                handleFileDownload(client, itemPath, msg);  // Handle file download
            }

            // itemPath = folderPath;
        }

        closedir(dir);
    }

    // Main handler function that determines if it's a file or folder
    static void handleDownload(ClientInfo& client, Message& msg) {
        std::string requestedPath = std::string(client.currentDir) + "/" + msg.payload;

        // Check if the requested path is a file or folder
        if (isDirectory(requestedPath.c_str())) {
            // It's a folder, process the folder
            handleFolderDownload(client, requestedPath, msg);
        } else {
            // It's a file, process the file download
            handleFileDownload(client, requestedPath, msg);
        } 
    }


    static void handleDataDown(ClientInfo& client, Message& msg) {
        if (client.file && client.bytesLeft > 0) {
            std::cout << "Handle Data Down - File is not empty" << std::endl;
            char buffer[PAYLOAD_SIZE];
            size_t bytesRead = fread(buffer, 1,
                std::min(static_cast<size_t>(PAYLOAD_SIZE), client.bytesLeft), client.file);

            msg.length = bytesRead;
            memcpy(msg.payload, buffer, bytesRead);
            client.bytesLeft -= bytesRead;

            if (client.bytesLeft == 0) {
                fclose(client.file);
                client.file = nullptr;
                send(client.socket, &msg, sizeof(msg), 0);
                msg.opcode = DOWNLOAD_SUCCESS;
                std::string message = "Download file " + std::string(client.filename);
                addToLog(message.c_str(), client.userId);
            }
        } else {
            std::cout << "Handle Data Down - File is empty" << std::endl;
            msg.length = 0;
        }
    }

    static void handleFileTransfer(int sock, Message& msg, ClientInfo& client) {
        switch (msg.opcode) {
            case UPLOAD:
                if (!PermissionHandler::checkPermission(client.currentDir, client.userId, WRITE)) {
                    msg.opcode = PERMISSION_DENIED;
                    return;
                }
                handleUpload(client, msg);
                break;
            case DATA_UP:
                if (!PermissionHandler::checkPermission(client.currentDir, client.userId, WRITE)) {
                    msg.opcode = PERMISSION_DENIED;
                    return;
                }
                handleDataUpload(client, msg);
                break;
            case DOWNLOAD:
                if (!PermissionHandler::checkPermission(client.currentDir, client.userId, READ)) {
                    msg.opcode = PERMISSION_DENIED;
                    return;
                }
                handleDownload(client, msg);
                break;
            case DATA_DOWN:
                if (!PermissionHandler::checkPermission(client.currentDir, client.userId, READ)) {
                    msg.opcode = PERMISSION_DENIED;
                    return;
                }
                handleDataDown(client, msg);
                break;
            case DELETE_FILE:
                if (!PermissionHandler::checkPermission(client.currentDir, client.userId, WRITE)) {
                    msg.opcode = PERMISSION_DENIED;
                    return;
                }
                handleDelete(client, msg);
                break;
            case SEARCH_FILE:
                if (!PermissionHandler::checkPermission(client.currentDir, client.userId, READ)) {
                    msg.opcode = PERMISSION_DENIED;
                    return;
                }
                handleSearchFile(client, msg);
                break;
        }
    }
};