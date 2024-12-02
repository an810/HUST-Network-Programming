#pragma once
#include "resource.h"

class Authentication {
public:
    static void loadAccounts(std::vector<Account>& accounts) {
        std::ifstream file("account.txt");
        Account acc;
        while (file >> acc.id >> acc.user >> acc.pass) {
            acc.status = 0;
            accounts.push_back(acc);
        }
        file.close();
    }

    static void handleLogin(int sock, Message& msg, std::vector<Account>& accounts) {
        char id[6], pass[25];
        sscanf(msg.payload, "%s %s", id, pass);

        for (auto& acc : accounts) {
            if (strcmp(acc.id, id) == 0) {
                if (strcmp(acc.pass, pass) == 0) {
                    acc.sock = sock;
                    acc.status = 1;
                    msg.opcode = LOGIN_SUCCESS;
                    return;
                }
                msg.opcode = WRONG_PASSWORD;
                return;
            }
        }
        msg.opcode = ID_NOT_FOUND;
    }
};