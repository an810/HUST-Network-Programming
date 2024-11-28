#pragma once
#include "resource.h"

class Authentication {
public:
    static bool login(int sock, const char* id, const char* pass) {
        Message msg;
        msg.opcode = LOGIN;
        snprintf(msg.payload, PAYLOAD_SIZE, "%s %s", id, pass);
        std::cout << msg.payload;
        send(sock, &msg, sizeof(Message), 0);
        recv(sock, &msg, sizeof(Message), 0);

        return msg.opcode == LOGIN_SUCCESS;
    }
};
