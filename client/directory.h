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
        std::cout << "Downloading folder: " << folderPath << std::endl;
        Message msg;
        msg.opcode = DOWNLOAD;
        strcpy(msg.payload, folderPath);
        send(sock, &msg, sizeof(Message), 0);
        recv(sock, &msg, sizeof(Message), 0);
        std::cout << "Server response: " << msg.opcode << std::endl;
        if (msg.opcode == FOLDER_NOT_FOUND) return false;
        
        std::string fullPath(CLIENT_FOLDER);
        fullPath += "/";
        fullPath += folderPath;
        
        if (msg.opcode == CREATE_FOLDER) {
            mkdir(fullPath.c_str(), 0777); // Create the folder on the client
        }


        // get filename from server
        recv(sock, &msg, sizeof(Message), 0);
        std::cout << "Received server response: " << msg.opcode << std::endl;
       
        std::cout << "Downloading file: " << msg.payload << std::endl;
        std::string filePath = fullPath + "/" + msg.payload;

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

        // Check for folder or file download
        recv(sock, &msg, sizeof(Message), 0);
        std::cout << "Download Folder - server response: " << msg.opcode << std::endl;

        if (msg.opcode == CREATE_FOLDER) {
            // Handle subfolder creation and download recursively
            // mkdir((fullPath + "/" + msg.payload).c_str(), 0777);
            std::cout << "Download Folder - subfolder name: " << msg.payload << std::endl;
            std::string nextPath = std::string(folderPath) + "/" + msg.payload;
            strcpy(msg.payload, nextPath.c_str());
            downloadFolder(sock, msg.payload);
        }

        return true;
    }

    // static bool downloadFolder(int sock, const char* folderPath) {
    //     std::cout << "Downloading folder: " << folderPath << std::endl;
    //     Message msg;

    //     // Send initial request for the folder
    //     msg.opcode = DOWNLOAD;
    //     strcpy(msg.payload, folderPath);
    //     send(sock, &msg, sizeof(Message), 0);

    //     // Create the folder locally
    //     recv(sock, &msg, sizeof(Message), 0);
    //     if (msg.opcode == FOLDER_NOT_FOUND) {
    //         std::cerr << "Folder not found: " << folderPath << std::endl;
    //         return false;
    //     }

    //     std::string fullPath(CLIENT_FOLDER);
    //     fullPath += "/";
    //     fullPath += folderPath;

    //     if (msg.opcode == CREATE_FOLDER) {
    //         mkdir(fullPath.c_str(), 0777); // Create the folder locally
    //     }

    //     // Process folder contents
    //     while (true) {
    //         recv(sock, &msg, sizeof(Message), 0);
    //         std::cout << "Server response: " << msg.opcode << std::endl;
    //         if (msg.opcode == DOWNLOAD_SUCCESS) {
    //             // End of folder contents
    //             break;
    //         }

    //         std::string itemName = msg.payload;
    //         std::string itemPath = fullPath + "/" + itemName;

    //         if (msg.opcode == CREATE_FOLDER) {
    //             // Handle subfolder
    //             mkdir(itemPath.c_str(), 0777); // Create the subfolder locally
    //             std::string nextFolderPath = std::string(folderPath) + "/" + itemName;

    //             // Recursive call to download the subfolder
    //             if (!downloadFolder(sock, nextFolderPath.c_str())) {
    //                 std::cerr << "Failed to download subfolder: " << nextFolderPath << std::endl;
    //                 return false;
    //             }
    //         } else if (msg.opcode == DATA_DOWN) {
                
    //             // get filename from server
    //             recv(sock, &msg, sizeof(Message), 0);
    //             std::cout << "Received server response: " << msg.opcode << std::endl;
            
    //             std::cout << "Downloading file: " << msg.payload << std::endl;
    //             std::string filePath = fullPath + "/" + msg.payload;

    //             // get the file content
    //             recv(sock, &msg, sizeof(Message), 0);

    //             // Handle file download
    //             FILE* file = fopen(itemPath.c_str(), "wb");
    //             if (!file) {
    //                 std::cerr << "Failed to create file: " << itemPath << std::endl;
    //                 return false;
    //             }

    //             // Receive file data
    //             do {
    //                 fwrite(msg.payload, 1, msg.length, file);
    //                 if (msg.length < PAYLOAD_SIZE) break;

    //                 msg.opcode = DATA_DOWN;
    //                 send(sock, &msg, sizeof(Message), 0);
    //                 recv(sock, &msg, sizeof(Message), 0);
    //             } while (msg.length > 0);

    //             fclose(file);
    //         } else {
    //             std::cerr << "Unexpected message opcode: " << msg.opcode << std::endl;
    //             return false;
    //         }
    //     }

    //     std::cout << "Completed downloading folder: " << folderPath << std::endl;
    //     return true;
    // }



    static void listFiles(int sock) {
        Message msg;
        msg.opcode = LIST_FILES;
        send(sock, &msg, sizeof(Message), 0);
        recv(sock, &msg, sizeof(Message), 0);
        std::cout << "Server response: " << msg.opcode << std::endl;
        std::cout << "*--------------------------------------------------\n";
        std::cout << "Files:\n" << msg.payload;
    }

    static bool changeDir(int sock, const char* dir) {
        Message msg;
        msg.opcode = CHANGE_DIR;
        strcpy(msg.payload, dir);
        send(sock, &msg, sizeof(Message), 0);
        recv(sock, &msg, sizeof(Message), 0);
        std::cout << "Server response: " << msg.opcode << std::endl;
        std::cout << "Current directory: " << msg.payload << std::endl;
        return msg.opcode == CHANGE_SUCCESS;
    }

    static bool createFolder(int sock, const char* name) {
        Message msg;
        msg.opcode = CREATE_FOLDER;
        strcpy(msg.payload, name);
        send(sock, &msg, sizeof(Message), 0);
        recv(sock, &msg, sizeof(Message), 0);
        std::cout << "Server response: " << msg.opcode << std::endl;
        return msg.opcode == CREATE_FOLDER_SUCCESS;
    }

    static bool deleteFolder(int sock, const char* name) {
        Message msg;
        msg.opcode = DELETE_FOLDER;
        strcpy(msg.payload, name);
        send(sock, &msg, sizeof(Message), 0);
        recv(sock, &msg, sizeof(Message), 0);
        std::cout << "Server response: " << msg.opcode << std::endl;
        return msg.opcode == DELETE_FOLDER_SUCCESS;
    }

    static bool renameFolder(int sock, const char* oldName, const char* newName) {
        Message msg;
        msg.opcode = RENAME_FOLDER;
        snprintf(msg.payload, PAYLOAD_SIZE, "%s %s", oldName, newName);
        send(sock, &msg, sizeof(Message), 0);
        recv(sock, &msg, sizeof(Message), 0);
        std::cout << "Server response: " << msg.opcode << std::endl;
        return msg.opcode == CREATE_FOLDER_SUCCESS;
    }
};