#pragma once
#include "resource.h"
#include "logger.h"

class Authentication {
private:
    static std::string generateId() {
        std::string id;
        std::ifstream file("pvc/account.txt");
        std::string maxId = "0000";
        std::string fileId, username, password;

        // Find max id from existing accounts
        while (file >> fileId >> username >> password) {
            if (fileId > maxId) {
                maxId = fileId;
            }
        }
        file.close();

        // Increment max id
        int newId = std::stoi(maxId) + 1;
        id = std::to_string(newId);

        // Pad with leading zeros to make 4 digits
        while (id.length() < 4) {
            id = "0" + id;
        }

        return id;
    }

    static bool validateRegistrationData(const char* username, const char* password) {
        // Kiểm tra độ dài tối thiểu
        if (strlen(username) < 3 || strlen(password) < 6) {
            return false;
        }

        // Kiểm tra username chỉ chứa ký tự và số
        for (int i = 0; username[i]; i++) {
            if (!isalnum(username[i])) return false;
        }

        return true;
    }

    static bool userExists(const char* username) {
        std::ifstream file("pvc/account.txt");
        std::string id, fileUsername, password;

        while (file >> id >> fileUsername >> password) {
            if (fileUsername == username) {
                file.close();
                return true;
            }
        }
        file.close();
        return false;
    }

public:

    static void handleRegistration(int sock, Message& msg) {
        char username[25], password[25];
        sscanf(msg.payload, "%s %s", username, password);

        // Validate registration data
        if (!validateRegistrationData(username, password)) {
            msg.opcode = INVALID_DATA;
            return;
        }

        // Check if username exists
        if (userExists(username)) {
            msg.opcode = USER_EXISTS;
            return;
        }

        // Generate new ID
        std::string newId = generateId();

        // Save new account - ensure each account on new line
        std::ofstream file("pvc/account.txt", std::ios::app);
        file << newId << " " << username << " " << password << std::endl;
        file.close();

        // Return registration info to user
        msg.opcode = REGISTER_SUCCESS;
        snprintf(msg.payload, PAYLOAD_SIZE, "%s %s %s", newId.c_str(), username);

        // Log registration
        std::string logMessage = "New user registered - Username: " + std::string(username) + ", ID: " + newId;
        Logger::addToLog(logMessage.c_str(), newId.c_str());
    }

    static void saveAccounts(const std::vector<Account>& accounts) {
        std::ofstream file("pvc/account.txt");
        for (const auto& acc : accounts) {
            file << acc.id << " " << acc.user << " " << acc.pass << " " << std::endl;
        }
        file.close();
    }

    static void loadAccounts(std::vector<Account>& accounts) {
        std::ifstream file("pvc/account.txt");
        Account acc;
        while (file >> acc.id >> acc.user >> acc.pass) {
            accounts.push_back(acc);
        }
        file.close();
    }

    static void handleLogin(int sock, Message& msg, std::vector<Account>& accounts, ClientInfo& client) {
        char username[25], password[25];
        sscanf(msg.payload, "%s %s", username, password);

        std::cout << "Login: " << username << " " << password << std::endl;

        for (const auto& acc : accounts) {
            std::cout << acc.user << " " << acc.pass << std::endl;
            if (strcmp(acc.user, username) == 0) {
                if (strcmp(acc.pass, password) == 0) {
                    msg.opcode = LOGIN_SUCCESS;
                    strcpy(client.userId, acc.user);
                    strcpy(msg.payload, acc.id);
                    return;
                } else {
                    msg.opcode = WRONG_PASSWORD;
                    return;
                }
            }
        }
        msg.opcode = ID_NOT_FOUND;
    }
};