#pragma once
#include "resource.h"

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

public:
    static bool checkPermission(const char* path, const char* userId, PermissionType requiredPermission) {
        // Admin có tất cả quyền
        if (strcmp(userId, "admin") == 0) return true;

        for (const auto& perm : permissions) {
            if !(strcmp(perm.userId, userId) == 0) {
                continue;
            }
            std::cout << "Checking path: " << path << " " << perm.path << std::endl;
            if (strcmp(path, perm.path) == 0 && strcmp(perm.userId, userId) == 0) {
                std::cout << "Checking permission: " << perm.permissionType << " " << requiredPermission << std::endl;
                return (perm.permissionType & requiredPermission) != 0;
            }
        }
        return false;
    }

    static void addPermission(const char* path, const char* userId, int permType) {
        Permission perm;
        strcpy(perm.path, path);
        strcpy(perm.userId, userId);
        perm.permissionType = permType;
        permissions.push_back(perm);
        savePermissions();
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

std::vector<Permission> PermissionHandler::permissions;