#pragma once
#include "resource.h"
#include "logger.h"

class PermissionHandler {
private:
    static std::vector<Permission> permissions;

    static void loadPermissions() {
        std::ifstream file("permissions.txt");
        Permission perm;
        while (file >> perm.path >> perm.userId >> perm.permissionType) {
            permissions.push_back(perm);
        }
        file.close();
    }

    static void savePermissions() {
        std::ofstream file("permissions.txt");
        for (const auto& perm : permissions) {
            file << perm.path << " " << perm.userId << " " << perm.permissionType << std::endl;
        }
        file.close();
    }

    static bool isSubPath(const std::string& parentPath, const std::string& childPath) {
        if (parentPath.empty() || childPath.empty()) return false;
        if (childPath == parentPath) return true;
        return childPath.find(parentPath + "/") == 0;
    }

public:
    static bool checkPermission(const char* path, const char* userId, PermissionType requiredPermission) {
        if (strcmp(userId, "admin") == 0) return true;

        for (const auto& perm : permissions) {
            if (strcmp(perm.userId, userId) != 0) continue;

            std::string pathStr(path);
            std::string permPathStr(perm.path);

            if (isSubPath(permPathStr, pathStr)) {
                bool hasPermission = (perm.permissionType & requiredPermission) != 0;
                std::cout << "Permission check: " << userId
                          << " requesting " << requiredPermission
                          << " on " << path
                          << " -> " << (hasPermission ? "granted" : "denied")
                          << std::endl;
                return hasPermission;
            }
        }
        std::cout << "Permission denied: No matching permission found for "
                  << userId << " on " << path << std::endl;
        return false;
    }

    static void addPermission(const char* path, const char* userId, int permType) {
        // First try to find and update existing permission
        for (auto& perm : permissions) {
            if (strcmp(perm.path, path) == 0 && strcmp(perm.userId, userId) == 0) {
                perm.permissionType = permType;
                savePermissions();
                return;
            }
        }

        // If not found, add new permission
        Permission perm;
        strcpy(perm.path, path);
        strcpy(perm.userId, userId);
        perm.permissionType = permType;
        permissions.push_back(perm);
        savePermissions();
    }

    static void handleGrantPermission(ClientInfo &client, Message &msg) {
        char path[256];
        char userId[25];
        int permission;

        try {
            std::cout << "Grant permission " << msg.payload << std::endl;

            // Parse message payload
            if (sscanf(msg.payload, "%s %s %d", path, userId, &permission) != 3) {
                msg.opcode = PERMISSION_DENIED;
                return;
            }

            // Validate permission
            if (permission < 1 || permission > 7) {
                msg.opcode = PERMISSION_DENIED;
                return;
            }

            // Create full path
            std::string fullPath;
            if (path[0] == '/') {
                fullPath = std::string(SERVER_FOLDER) + path;
            } else {
                fullPath = std::string(SERVER_FOLDER) + "/" + path;
            }

            // Check path exists
            struct stat st;
            if (stat(fullPath.c_str(), &st) != 0) {
                msg.opcode = FOLDER_NOT_FOUND;
                return;
            }

            // Grant permission
            addPermission(fullPath.c_str(), userId, permission);

            msg.opcode = GRANT_SUCCESS;
            std::cout << "Finish grant permission" << std::endl;
            // Log activity
            std::string logMessage = "Granted permission " + std::to_string(permission) +
                                 " to user " + std::string(userId) +
                                 " for path " + fullPath;
            Logger::addToLog(logMessage.c_str(), client.userId);

        } catch (const std::exception& e) {
            msg.opcode = PERMISSION_DENIED;
        }

    }

    static void revokePermission(const char* path, const char* userId) {
        permissions.erase(
            std::remove_if(permissions.begin(), permissions.end(),
                [path, userId](const Permission& p) {
                    return strcmp(p.path, path) == 0 && strcmp(p.userId, userId) == 0;
                }),
            permissions.end()
        );
        savePermissions();
    }

    static void init() {
        loadPermissions();
    }
};

// Define static members
std::vector<Permission> PermissionHandler::permissions;