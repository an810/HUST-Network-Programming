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
        
        std::string folderName(CLIENT_FOLDER);
        folderName += "/";
        folderName += folderPath;
        
        if (msg.opcode == CREATE_FOLDER) {
            mkdir(folderName.c_str(), 0777); // Create the folder on the client
        }


        // while (msg.opcode == DATA_DOWN) {
            std::cout << "Downloading file: " << msg.payload << std::endl;
            std::string filePath = folderName + "/" + msg.payload;
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

            if (msg.opcode == CREATE_FOLDER) {
                // Handle subfolder creation and download recursively
                mkdir((folderName + "/" + msg.payload).c_str(), 0777);
                downloadFolder(sock, msg.payload);
            }
        // }

        return true;
    }

    // static void downloadFolder(int serverSocket, const std::string &targetPath) {
    //     mkdir(targetPath.c_str(), 0777); // Create the base folder locally

    //     Message msg;
    //     while (true) {
    //         recv(serverSocket, &msg, sizeof(msg), 0);

    //         if (msg.opcode == FOLDER_NOT_FOUND) {
    //             std::cout << "Folder not found!" << std::endl;
    //             break;
    //         } else if (msg.opcode == CREATE_FOLDER) {
    //             // Create a subfolder
    //             std::string folderPath = targetPath + "/" + msg.payload;
    //             mkdir(folderPath.c_str(), 0777);
    //             std::cout << "Created folder: " << folderPath << std::endl;
    //         } else if (msg.opcode == DATA_DOWN) {
    //             // Handle file download
    //             std::string filePath = targetPath + "/" + msg.payload;
    //             FILE *file = fopen(filePath.c_str(), "wb");
    //             if (!file) {
    //                 std::cerr << "Failed to create file: " << filePath << std::endl;
    //                 continue;
    //             }

    //             do {
    //                 fwrite(msg.payload, 1, msg.length, file);
    //                 if (msg.length < PAYLOAD_SIZE)
    //                     break; // End of file

    //                 // Request the next chunk
    //                 send(serverSocket, &msg, sizeof(msg), 0);
    //                 recv(serverSocket, &msg, sizeof(msg), 0);
    //             } while (msg.opcode == DATA_DOWN);

    //             fclose(file);
    //             std::cout << "Downloaded file: " << filePath << std::endl;
    //         } else if (msg.opcode == DOWNLOAD_SUCCESS) {
    //             std::cout << "Folder download complete!" << std::endl;
    //             break;
    //         } else {
    //             std::cout << "Unknown opcode: " << msg.opcode << std::endl;
    //             break;
    //         }
    //     }
    // }

    // static bool downloadFolder(int sock, const char* folderPath) {
    //     Message msg;
    //     msg.opcode = DOWNLOAD;
    //     strcpy(msg.payload, folderPath);
    //     send(sock, &msg, sizeof(Message), 0);
    //     recv(sock, &msg, sizeof(Message), 0);

    //     if (msg.opcode != CREATE_FOLDER) {
    //         std::cout << "Server failed to start folder download" << std::endl;
    //         return false;
    //     }

    //     // Start processing the incoming folder data
    //     std::string basePath = std::string(CLIENT_FOLDER) + "/" + folderPath;
    //     mkdir(basePath.c_str(), 0777); // Create the base folder locally

    //     while (true) {
    //         recv(sock, &msg, sizeof(Message), 0);

    //         if (msg.opcode == FOLDER_NOT_FOUND) {
    //             std::cout << "Folder not found" << std::endl;
    //             break;
    //         } else if (msg.opcode == CREATE_FOLDER) {
    //             std::string subFolderPath = basePath + "/" + msg.payload;
    //             std::cout << "Creating folder: " << subFolderPath << std::endl;
    //             mkdir(subFolderPath.c_str(), 0777);
    //         } else if (msg.opcode == DATA_DOWN) {
    //             // Server informs a file download will begin
    //             std::string filePath = basePath + "/" + msg.payload;
    //             std::cout << "Downloading file - payload: " << msg.payload << std::endl;
    //             std::cout << "Downloading file: " << filePath << std::endl;
    //             if (!downloadFileToFolder(sock, basePath, msg.payload)) {
    //                 std::cout << "Failed to download file: " << filePath << std::endl;
    //                 break;
    //             }
    //         } else {
    //             std::cout << "Unknown opcode received: " << msg.opcode << std::endl;
    //             break;
    //         }
    //     }

    //     recv(sock, &msg, sizeof(Message), 0);
    //     std::cout << "Server response: " << msg.opcode << std::endl;
    //     if (msg.opcode == FOLDER_NOT_FOUND) {
    //         std::cout << "Folder not found" << std::endl;
    //         return false;
    //     }
    //     return msg.opcode == DOWNLOAD_SUCCESS;
    // }


    static bool downloadFileToFolder(int sock, const std::string& folderPath, const char* filename) {
        // Construct the full path for the file within the "Download" folder
        std::string targetFolder = folderPath;
        std::string filepath = targetFolder + "/" + filename;

        // Ensure the folder exists (create it if not)
        mkdir(targetFolder.c_str(), 0777);

        // Request file download from the server
        Message msg;
        // msg.opcode = DOWNLOAD;
        // strcpy(msg.payload, filename);

        // send(sock, &msg, sizeof(Message), 0);
        recv(sock, &msg, sizeof(Message), 0);

        if (msg.opcode != DATA_DOWN) {
            std::cout << "Failed to start file download for: " << filename << std::endl;
            return false;
        }

        FILE* file = fopen(filepath.c_str(), "wb");
        if (!file) {
            std::cerr << "Failed to create file: " << filepath << std::endl;
            return false;
        }

        // Receive the file data in chunks
        do {
            fwrite(msg.payload, 1, msg.length, file);
            if (msg.length < PAYLOAD_SIZE) break; // End of file

            // Request the next chunk
            msg.opcode = DATA_DOWN;
            send(sock, &msg, sizeof(Message), 0);
            recv(sock, &msg, sizeof(Message), 0);
        } while (msg.length > 0);

        fclose(file);
        recv(sock, &msg, sizeof(Message), 0);

        std::cout << "Download completed for: " << filepath << ", Server response: " << msg.opcode << std::endl;
        return msg.opcode == DOWNLOAD_SUCCESS;
    }


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