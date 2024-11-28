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
};