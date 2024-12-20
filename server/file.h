#pragma once
#include "resource.h"

class FileHandler {
public:
    static void handleDelete(ClientInfo& client, Message& msg) {
        std::cout << "Delete file - Client current dir: " << client.currentDir << std::endl;
        std::string filepath = std::string(client.currentDir) + "/" + msg.payload;
        std::cout << "Delete file - Filepath: " << filepath << std::endl;
        if (remove(filepath.c_str()) == 0) {
            msg.opcode = DELETE_FILE_SUCCESS;
            std::cout << "Delete file - File deleted successfully " << msg.opcode << std::endl;
        } else {
            msg.opcode = FILE_NOT_FOUND;
            std::cout << "Delete file - File not found " << msg.opcode << std::endl;
        }

        send(client.socket, &msg, sizeof(msg), 0);
    }


    static void searchInSubfolders(const std::string& basePath, const std::string& fileName, std::string& foundPath) {
        DIR* dir = opendir(basePath.c_str());
        if (!dir) return;

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

            std::string path = basePath + "/" + entry->d_name;
            if (isDirectory(path.c_str())) {
                searchInSubfolders(path, fileName, foundPath);
                if (!foundPath.empty()) break;
            } else if (strcmp(entry->d_name, fileName.c_str()) == 0) {
                foundPath = path;
                break;
            }
        }
        closedir(dir);
    }

    static void handleSearchFile(ClientInfo& client, Message& msg) {
        std::string foundPath;
        searchInSubfolders(SERVER_FOLDER, msg.payload, foundPath);

        if (!foundPath.empty()) {
            strcpy(msg.payload, foundPath.c_str());
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
            }
        }
    }

    static void handleDownload(ClientInfo& client, Message& msg) {
        std::string filepath = std::string(client.currentDir) + "/" + msg.payload;
        client.file = fopen(filepath.c_str(), "rb");
        if (client.file) {
            fseek(client.file, 0, SEEK_END);
            client.fileSize = ftell(client.file);
            client.bytesLeft = client.fileSize;
            rewind(client.file);
            strcpy(client.filename, msg.payload);

            msg.opcode = DATA_DOWN;
            handleDataDown(client, msg);
        } else {
            msg.opcode = FILE_NOT_FOUND;
        }
    }

    static bool isDirectory(const char* path) {
        struct stat st;
        // string currentPath = string(SERVER_FOLDER) + "/" + path;
        if (stat(path, &st) == 0) {
            return S_ISDIR(st.st_mode);
        }
        return false;
    }

    


    static void handleFolderDownload(ClientInfo& client, Message& msg) {
        std::string targetPath = msg.payload;
        std::string path = std::string(client.currentDir) + "/" + targetPath;

        struct stat st;
        if (stat(path.c_str(), &st) != 0) {
            msg.opcode = FILE_NOT_FOUND;
            send(client.socket, &msg, sizeof(msg), 0);
            return;
        }

        // Check if the path is a directory or a file
        if (S_ISDIR(st.st_mode)) {
            // It's a directory, we need to send folder structure
            std::cout << "Create folder in client side" << std::endl;
            msg.opcode = CREATE_FOLDER;
            send(client.socket, &msg, sizeof(msg), 0);

            // Send files and subdirectories recursively
            DIR* dir = opendir(path.c_str());
            if (dir == nullptr) {
                std::cout << "Folder is empty" << std::endl; 
                msg.opcode = FOLDER_NOT_FOUND;
                send(client.socket, &msg, sizeof(msg), 0);
                return;
            }

            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                    std::string filePath = path + "/" + entry->d_name;
                    std::cout << "Handle Download - filePath: " << filePath << std::endl;
                    std::cout << "Handle Download - entry file name: " << entry->d_name << std::endl;
                    std::string nextPath = targetPath + "/" + entry->d_name;
                    if (isDirectory(filePath.c_str())) {
                        // Recursively handle subfolders
                        strcpy(msg.payload, nextPath.c_str());
                        std::cout << "Handle Download - recursive download subfolder" << std::endl;
                        handleDownload(client, msg); // Recursive call to handle subfolder
                    } else {
                        // Handle file download for each file in the directory
                        strcpy(msg.payload, nextPath.c_str());
                        std::cout << "Handle Download - download file" << std::endl;
                        if (!handleFileDownload(client, msg)) {
                            msg.opcode = FILE_NOT_FOUND;
                            return;
                        } 
                    }
                }
            }
            std::cout << "Handle Download - close directory" << std::endl;
            closedir(dir);
        } else {
            // It's a file, send the file to the client
            std::cout << "Handle Download - It's a file, send the file to the client" << std::endl;
            if (!handleFileDownload(client, msg)) {
                msg.opcode = FILE_NOT_FOUND;
            }
        }
    }

    // static void handleFileInFolderDownload(ClientInfo& client, Message& msg) {
    //     std::string filepath = std::string(client.currentDir) + "/" + msg.payload;
    //     client.file = fopen(filepath.c_str(), "rb");
    //     if (client.file) {
    //         fseek(client.file, 0, SEEK_END);
    //         client.fileSize = ftell(client.file);
    //         client.bytesLeft = client.fileSize;
    //         rewind(client.file);
    //         strcpy(client.filename, msg.payload);
    //     }
    // }

    static bool handleFileDownload(ClientInfo& client, Message& msg) {
        std::string filepath = std::string(client.currentDir) + "/" + msg.payload;
        client.file = fopen(filepath.c_str(), "rb");
        
        std::cout << "Handle File Download - Filepath: " << filepath << std::endl;
        if (client.file) {
            fseek(client.file, 0, SEEK_END);
            client.fileSize = ftell(client.file);
            client.bytesLeft = client.fileSize;
            rewind(client.file);
            strcpy(client.filename, msg.payload);
            
            // Start sending file in chunks
            msg.opcode = DATA_DOWN;
            std::cout << "Handle File Download - Start sending file in chunks" << std::endl;
            handleDataDown(client, msg);
            std::cout << "Handle File Download - End sending file in chunks" << std::endl;
            send(client.socket, &msg, sizeof(msg), 0);
            return true;
        } else {
            send(client.socket, &msg, sizeof(msg), 0);
            return false;
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
            }
        } else {
            std::cout << "Handle Data Down - File is empty" << std::endl;
            msg.length = 0;
        }
    }

    static void handleFileTransfer(int sock, Message& msg, ClientInfo& client) {
        switch (msg.opcode) {
            case UPLOAD:
                handleUpload(client, msg);
                break;
            case DATA_UP:
                handleDataUpload(client, msg);
                break;
            case DOWNLOAD:
                handleDownload(client, msg);
                break;
            case DATA_DOWN:
                handleDataDown(client, msg);
                break;
            case DELETE_FILE:
                handleDelete(client, msg);
                break;
            case SEARCH_FILE:
                handleSearchFile(client, msg);
                break;
        }
    }
};