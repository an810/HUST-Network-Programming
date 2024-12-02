#pragma once
#include "resource.h"

class FileHandler {
public:
    static void handleUpload(ClientInfo& client, Message& msg) {
        std::string filepath = std::string(SERVER_FOLDER) + "/" + msg.payload;
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
            }
        }
    }

    static void handleDownload(ClientInfo& client, Message& msg) {
        std::string filepath = std::string(SERVER_FOLDER) + "/" + msg.payload;
        client.file = fopen(filepath.c_str(), "rb");
        if (client.file) {
            fseek(client.file, 0, SEEK_END);
            client.fileSize = ftell(client.file);
            client.bytesLeft = client.fileSize;
            rewind(client.file);
            strcpy(client.filename, msg.payload);
        }
    }

    static void handleDataDown(ClientInfo& client, Message& msg) {
        if (client.file && client.bytesLeft > 0) {
            char buffer[PAYLOAD_SIZE];
            size_t bytesRead = fread(buffer, 1,
                std::min(static_cast<size_t>(PAYLOAD_SIZE), client.bytesLeft), client.file);

            msg.length = bytesRead;
            memcpy(msg.payload, buffer, bytesRead);
            client.bytesLeft -= bytesRead;

            if (client.bytesLeft == 0) {
                fclose(client.file);
                client.file = nullptr;
            }
        } else {
            msg.length = 0;
        }
    }

    static void handleFileTransfer(int sock, Message& msg, std::vector<ClientInfo>& clients) {
        ClientInfo* client = nullptr;
        for (auto& c : clients) {
            if (c.socket == sock) {
                client = &c;
                break;
            }
        }

        if (!client) return;

        switch (msg.opcode) {
            case UPLOAD:
                handleUpload(*client, msg);
            break;
            case DATA_UP:
                handleDataUpload(*client, msg);
            break;
            case DOWNLOAD:
                handleDownload(*client, msg);
                if (client->file) {
                    msg.opcode = DATA_DOWN;
                    handleDataDown(*client, msg);
                } else {
                    msg.opcode = FOLDER_ERROR;
                    msg.length = 0;
                }
            break;
            case DATA_DOWN:
                handleDataDown(*client, msg);
            break;
        }
    }
};