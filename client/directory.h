#pragma once
#include "resource.h"
#include "file.h"

class Directory {
public:
    static bool isDirectory(const char* path) {
        struct stat st;
        std::string currentPath = std::string(CLIENT_FOLDER) + "/" + path;
        if (stat(currentPath.c_str(), &st) == 0) {
            return S_ISDIR(st.st_mode);
        }
        return false;
    }

    
    static bool uploadFolder(int sock, const char* folderName) {
        std::string path = std::string(CLIENT_FOLDER) + "/" + folderName;    
        DIR *dir = opendir(path.c_str());
        if (!dir) return false;

        Message msg;
        msg.opcode = CREATE_FOLDER;
        strcpy(msg.payload, folderName);
        send(sock, &msg, sizeof(Message), 0);
        recv(sock, &msg, sizeof(Message), 0);

        struct dirent* entry;
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] == '.') continue;

            std::string filePath = std::string(path) + "/" + entry->d_name;
            std::string fileName;
            
            // Find the position of "ClientData" in the filePath
            size_t pos = filePath.find(CLIENT_FOLDER_NAME);
            if (pos != std::string::npos) {
                // Truncate the filePath to remove "ClientData" and everything before it
                fileName = filePath.substr(pos + strlen(CLIENT_FOLDER_NAME));
            }

            std::cout << "Uploading: " << filePath << std::endl;
            std::cout << "File name: " << fileName << std::endl;
            std::cout << "Is directory: " << isDirectory(fileName.c_str()) << std::endl;
            if (isDirectory(fileName.c_str())) {
    
                msg.opcode = CREATE_FOLDER;
                std::cout << "Creating folder: " << entry->d_name << std::endl;
                strcpy(msg.payload, fileName.c_str());
                send(sock, &msg, sizeof(Message), 0);
                recv(sock, &msg, sizeof(Message), 0);
                uploadFolder(sock, fileName.c_str());
            } else {
                FileHandler::uploadFile(sock, fileName.c_str());
            }
        }

        closedir(dir);

        // recv(sock, &msg, sizeof(Message), 0);
        std::cout << "Server response - uploadFolder: " << msg.opcode << std::endl;
        return true;
    }

    static bool downloadFolder(int sock, const char* folderPath) {
        std::cout << "Start downloading folder: " << folderPath << std::endl;
        Message msg;

        // Notify server to start downloading the folder
        msg.opcode = DOWNLOAD;
        strcpy(msg.payload, folderPath);
        send(sock, &msg, sizeof(Message), 0);

        // Wait for server response
        recv(sock, &msg, sizeof(Message), 0);
        if (msg.opcode == PERMISSION_DENIED) {
            std::cout << "PERMISSION DENIED!" << std::endl;
            return false;
        }
        std::cout << "Start downloading folder - Server response: " << msg.opcode << " - " << msg.payload << std::endl;

        if (msg.opcode == FOLDER_NOT_FOUND) return false;

        std::string fullPath(CLIENT_FOLDER);
        fullPath += "/";
        fullPath += folderPath;

        // Create the folder locally
        if (msg.opcode == CREATE_FOLDER) {
            mkdir(fullPath.c_str(), 0777);
            std::cout << "Creating folder: " << fullPath << std::endl;
        }

        while (true) {
            recv(sock, &msg, sizeof(Message), 0);

            std::cout << "Server response: " << msg.opcode << " - " << msg.payload << std::endl;

            if (msg.opcode == DOWNLOAD_SUCCESS) {
                std::cout << "Finished downloading folder: " << folderPath << std::endl;
                break;  // Folder download complete
            }

            if (msg.opcode == FOLDER_NOT_FOUND || msg.opcode == FILE_NOT_FOUND) {
                std::cout << "Folder/File not found: " << msg.payload << std::endl;
                break;
            }

            if (msg.opcode == CREATE_FOLDER) {
                // Handle subfolder
                std::string subFolderPath = fullPath + "/" + msg.payload;
                std::cout << "Creating subfolder: " << subFolderPath << std::endl;
                mkdir(subFolderPath.c_str(), 0777);

                // Recursively download the subfolder
                fullPath += "/";
                fullPath += msg.payload;
                std::cout << "Download Folder - Subfolder: " << fullPath << std::endl;
                // std::string nextPath = std::string(folderPath) + "/" + msg.payload;
                // downloadFolder(sock, nextPath.c_str());
            } else if (msg.opcode == DATA_DOWN) {
                // Handle file download
                
                std::string filePath = fullPath + "/" + msg.payload;
                std::cout << "Downloading file: " << filePath << std::endl;

                // get the file content
                recv(sock, &msg, sizeof(Message), 0);
            
                FILE* file = fopen(filePath.c_str(), "wb");
                if (!file) return false;

                std::cout << "Server response: " << msg.opcode << std::endl;
                if (msg.opcode != DATA_DOWN) return false;

                do {
                    std::cout << "Start write file" << std::endl;
                    fwrite(msg.payload, 1, msg.length, file);
                    if (msg.length < PAYLOAD_SIZE) break;
                
                    msg.opcode = DATA_DOWN;
                    send(sock, &msg, sizeof(Message), 0);
                    recv(sock, &msg, sizeof(Message), 0);
                } while (msg.length > 0);

                fclose(file);
            } else
                return false;
        }
        return true;
    }



    static void listFiles(int sock) {
        Message msg;
        msg.opcode = LIST_FILES;
        send(sock, &msg, sizeof(Message), 0);
        recv(sock, &msg, sizeof(Message), 0);
        std::cout << "Server response: " << msg.opcode << std::endl;
        if (msg.opcode == PERMISSION_DENIED) std::cout << "PERMISSION DENIED!\n";
        if (msg.opcode == LIST_FILES) {
            std::cout << "*--------------------------------------------------\n";
            std::cout << "Files:\n" << msg.payload;
        }
    }

    static bool changeDir(int sock, const char* dir) {
        Message msg;
        msg.opcode = CHANGE_DIR;
        strcpy(msg.payload, dir);
        send(sock, &msg, sizeof(Message), 0);
        recv(sock, &msg, sizeof(Message), 0);
        std::cout << "Server response: " << msg.opcode << std::endl;
        if (msg.opcode == PERMISSION_DENIED) {
            std::cout << "PERMISSION DENIED!\n";
        }
        else {
            std::cout << "Current directory: " << msg.payload << std::endl;
        }
        return msg.opcode == CHANGE_SUCCESS;
    }

    static bool createFolder(int sock, const char* name) {
        Message msg;
        msg.opcode = CREATE_FOLDER;
        strcpy(msg.payload, name);
        send(sock, &msg, sizeof(Message), 0);
        recv(sock, &msg, sizeof(Message), 0);
        std::cout << "Server response: " << msg.opcode << std::endl;
        if (msg.opcode == PERMISSION_DENIED) std::cout << "PERMISSION DENIED!\n";
        return msg.opcode == CREATE_FOLDER_SUCCESS;
    }

    static bool deleteFolder(int sock, const char* name) {
        Message msg;
        msg.opcode = DELETE_FOLDER;
        strcpy(msg.payload, name);
        send(sock, &msg, sizeof(Message), 0);
        recv(sock, &msg, sizeof(Message), 0);
        std::cout << "Server response: " << msg.opcode << std::endl;
        if (msg.opcode == PERMISSION_DENIED) std::cout << "PERMISSION DENIED!\n";
        return msg.opcode == DELETE_FOLDER_SUCCESS;
    }

    static bool renameFolder(int sock, const char* oldName, const char* newName) {
        Message msg;
        msg.opcode = RENAME_FOLDER;
        snprintf(msg.payload, PAYLOAD_SIZE, "%s %s", oldName, newName);
        send(sock, &msg, sizeof(Message), 0);
        recv(sock, &msg, sizeof(Message), 0);
        std::cout << "Server response: " << msg.opcode << std::endl;
        if (msg.opcode == PERMISSION_DENIED) std::cout << "PERMISSION DENIED!\n";
        return msg.opcode == CREATE_FOLDER_SUCCESS;
    }
};