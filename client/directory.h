#pragma once
#include "resource.h"

class Directory {
public:
    static void listFiles(int sock) {
        Message msg;
        msg.opcode = LIST_FILES;
        send(sock, &msg, sizeof(Message), 0);
        recv(sock, &msg, sizeof(Message), 0);
        cout << "Files:\n" << msg.payload;
    }

    static bool changeDir(int sock, const char* dir) {
        Message msg;
        msg.opcode = CHANGE_DIR;
        strcpy(msg.payload, dir);
        send(sock, &msg, sizeof(Message), 0);
        recv(sock, &msg, sizeof(Message), 0);
        return true;
    }

    static bool createFolder(int sock, const char* name) {
        Message msg;
        msg.opcode = CREATE_FOLDER;
        strcpy(msg.payload, name);
        send(sock, &msg, sizeof(Message), 0);
        recv(sock, &msg, sizeof(Message), 0);
        return msg.opcode == CREATE_FOLDER_SUCCESS;
    }

    static bool deleteFolder(int sock, const char* name) {
        Message msg;
        msg.opcode = DELETE_FOLDER;
        strcpy(msg.payload, name);
        send(sock, &msg, sizeof(Message), 0);
        recv(sock, &msg, sizeof(Message), 0);
        return msg.opcode == CREATE_FOLDER_SUCCESS;
    }

    static bool renameFolder(int sock, const char* oldName, const char* newName) {
        Message msg;
        msg.opcode = RENAME_FOLDER;
        snprintf(msg.payload, PAYLOAD_SIZE, "%s %s", oldName, newName);
        send(sock, &msg, sizeof(Message), 0);
        recv(sock, &msg, sizeof(Message), 0);
        return msg.opcode == CREATE_FOLDER_SUCCESS;
    }
};