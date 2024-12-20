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
#include <dirent.h>
#include <iostream>


#define PAYLOAD_SIZE 1024
#define MESSAGE_SIZE 1036
#define BUFF_SIZE 2048
#define CLIENT_FOLDER "./ClientData"
#define CLIENT_FOLDER_NAME "ClientData/"


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
    FOLDER_NOT_FOUND = 143
};
