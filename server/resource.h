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
    int status;
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
    CHANGE_DIR = 39
};
