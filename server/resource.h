#pragma once
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <sys/stat.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <sys/select.h>
#include <time.h>

#define BUFF_SIZE 2048
#define MAX_CLIENTS 20
#define PAYLOAD_SIZE 1024
#define MESSAGE_SIZE 1036
#define SERVER_FOLDER "./ServerData"


struct Account {
    int sock;
    char id[6];
    char user[25];
    char pass[25];
};

struct Message {
    int opcode;
    int offset;
    int length;
    char payload[PAYLOAD_SIZE];
};

struct ClientInfo {
    int socket;
    FILE* file;
    size_t fileSize;
    size_t bytesLeft;
    char filename[200];
    char currentDir[256];
    char userId[25];
};

enum OpCodes {
    LOGIN = 10,
    LOGIN_SUCCESS = 100,
    ID_NOT_FOUND = 102,
    WRONG_PASSWORD = 104,

    REGISTER = 11,
    REGISTER_SUCCESS = 111,
    REGISTER_FAILED = 112,
    USER_EXISTS = 113,
    INVALID_DATA = 114,

    UPLOAD = 30,
    DOWNLOAD = 31,
    DATA_UP = 32,
    DATA_DOWN = 33,
    LIST_FILES = 34,
    SEARCH_FILE = 35,

    CHANGE_DIR = 39,

    CREATE_FOLDER = 40,
    DELETE_FOLDER = 41,
    RENAME_FOLDER = 42,
    CREATE_FILE = 43,
    DELETE_FILE = 44,
    RENAME_FILE = 45,

    UPLOAD_SUCCESS = 130,
    DOWNLOAD_SUCCESS = 131,
    CHANGE_SUCCESS = 132,
    
    SEARCH_FILE_SUCCESS = 133,
    CREATE_FILE_SUCCESS = 137,
    DELETE_FILE_SUCCESS = 138,
    FILE_NOT_FOUND = 139,

    CREATE_FOLDER_SUCCESS = 140,
    DELETE_FOLDER_SUCCESS = 141,
    FOLDER_ALREADY_EXIST = 142,
    FOLDER_NOT_FOUND = 143,

    PERMISSION_DENIED = 150,
    GRANT_PERMISSION = 46,
    GRANT_SUCCESS = 152,
};

enum PermissionType {
    READ = 1,      // Quyền đọc file/folder
    WRITE = 2,     // Quyền ghi/upload
    EXECUTE = 4,   // Quyền thực thi/truy cập folder
    FULL = 7       // Tất cả quyền
};

struct Permission {
    char path[256];        // Đường dẫn folder/file
    char userId[25];       // UserID được cấp quyền
    int permissionType;    // Loại quyền (dùng PermissionType)
};