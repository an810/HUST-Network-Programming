#pragma once
#include "resource.h"

class FileHandler {
public:
    static bool deleteFile(int sock, const char* filename) {
        Message msg;
        msg.opcode = DELETE_FILE;
        strcpy(msg.payload, filename);

        send(sock, &msg, sizeof(Message), 0);
        recv(sock, &msg, sizeof(Message), 0);

        std::cout << "Server response: " << msg.opcode << std::endl;
        return msg.opcode == DELETE_FILE_SUCCESS;
    }

    static bool searchFile(int sock, const char* filename) {
        Message msg;
        msg.opcode = SEARCH_FILE;
        strcpy(msg.payload, filename);

        send(sock, &msg, sizeof(Message), 0);
        recv(sock, &msg, sizeof(Message), 0);

        std::cout << "Server response: " << msg.opcode << std::endl;
        std::cout << "File found at: " << msg.payload << std::endl;
        return msg.opcode == SEARCH_FILE_SUCCESS;
    }

    static bool uploadFile(int sock, const char* filename) {
        std::string filepath = std::string(CLIENT_FOLDER) + "/" + filename;
        FILE* file = fopen(filepath.c_str(), "rb");
        if (!file) return false;

        Message msg;
        msg.opcode = UPLOAD;
        strcpy(msg.payload, filename);

        fseek(file, 0, SEEK_END);
        msg.length = ftell(file);
        rewind(file);

        send(sock, &msg, sizeof(Message), 0);
        recv(sock, &msg, sizeof(Message), 0);

        char buffer[PAYLOAD_SIZE];
        size_t bytesRead;
        int offset = 0;

        while ((bytesRead = fread(buffer, 1, PAYLOAD_SIZE, file)) > 0) {
            msg.opcode = DATA_UP;
            msg.offset = offset++;
            msg.length = bytesRead;
            memcpy(msg.payload, buffer, bytesRead);
            send(sock, &msg, sizeof(Message), 0);
            recv(sock, &msg, sizeof(Message), 0);
        }

        fclose(file);
        // recv(sock, &msg, sizeof(Message), 0);
        std::cout << "Server response - uploadFile: " << msg.opcode << std::endl;
        return msg.opcode == DATA_UP;
    }

    static bool downloadFile(int sock, const char* filename) {
        std::cout << "Downloading file: " << filename << std::endl;
        Message msg;
        msg.opcode = DOWNLOAD;
        strcpy(msg.payload, filename);

        send(sock, &msg, sizeof(Message), 0);
        recv(sock, &msg, sizeof(Message), 0);
        if (msg.opcode != DATA_DOWN) return false;

        std::string filepath = std::string(CLIENT_FOLDER) + "/" + filename;
        FILE* file = fopen(filepath.c_str(), "wb");
        if (!file) return false;

        do {
            fwrite(msg.payload, 1, msg.length, file);
            if (msg.length < PAYLOAD_SIZE) break;

            msg.opcode = DATA_DOWN;
            send(sock, &msg, sizeof(Message), 0);
            recv(sock, &msg, sizeof(Message), 0);
        } while (msg.length > 0);

        fclose(file);   
        recv(sock, &msg, sizeof(Message), 0);
        std::cout << "Server response: " << msg.opcode << std::endl;
        return msg.opcode == DOWNLOAD_SUCCESS;
    }
};