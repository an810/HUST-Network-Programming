#pragma once
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <cstring>
#include <filesystem>
#include <sys/stat.h>

using namespace std;

#define PAYLOAD_SIZE 1024
#define MESSAGE_SIZE 1036
#define BUFF_SIZE 2048
#define CLIENT_FOLDER "./ClientData"

struct Message {
    int opcode;
    int offset;
    int length;
    char payload[PAYLOAD_SIZE];
};

enum OpCodes {
    LOGIN = 10,
    LOGIN_SUCCESS = 100,
    ID_NOT_FOUND = 102,
    WRONG_PASSWORD = 104,

    UPLOAD = 30,
    DOWNLOAD = 31,
    DATA_UP = 32,
    DATA_DOWN = 33,
    LIST_FILES = 34,
    CHANGE_DIR = 39,

    CREATE_FOLDER = 40,
    DELETE_FOLDER = 41,
    RENAME_FOLDER = 42,

    CREATE_FOLDER_SUCCESS = 140,
    FOLDER_ERROR = 143
};
