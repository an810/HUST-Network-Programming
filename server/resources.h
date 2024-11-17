#pragma once

#include <iostream>
#include <string.h>
#include <vector>
#include <fstream>


using namespace std;



#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 5500
#define BACKLOG 20
#define RECEIVE 0 
#define SEND 1

#define BUFF_SIZE 2048
#define MAX_CLIENT 100
#define PAYLOAD_SIZE 1024
#define MESSAGE_SIZE 1024

#define FILE_ACCOUNT "server/account.txt"
#define DELIMITER " "
#define SERVER_FOLDER "./server_data"
#define CLIENT_FOLDER "./client_data"




typedef struct Account {
    

}Account;

typedef struct Message {
    int opcode;
    unsigned char permission;
    int offset;
    int length;
    char payload[PAYLOAD_SIZE];
}Message;

enum ACCOUNT_REQUEST {
    LOGIN = 10, 
    LOGOUT = 11,
    REGISTER = 12,
};

enum ACCOUNT_RESPONSE {
    LOGIN_SUCCESS = 100,
    LOGOUT_SUCCESS = 101,

    WRONG_INPUT = 102,
    ID_NOT_FOUND = 103,
    WRONG_PASSWORD = 104,
    BLOCKED = 105,

    ACCOUNT_EXIST = 106,
    REGISTER_SUCCESS = 107,
};

enum FILE_REQUEST {
    UPLOAD = 20,
    DOWNLOAD = 21,
    DATA_UP = 22,
    DATA_DOWN = 23,

    LIST_FILE = 24,
    DELETE_FILE = 25,
    CREATE_FOLDER = 26,
    DELETE_FOLDER = 27,
    

    CHANGE_DIRECTORY = 28,
    SHOW_LOG = 29,
};

enum FILE_RESPONSE {
    ACCEPT_UPLOAD = 120,
    UPLOAD_SUCCESS = 121,
    ACCEPT_DOWNLOAD = 122,
    DOWNLOAD_SUCCESS = 123,

    DELETE_FILE_SUCCESS = 124,
    CREATE_FOLDER_SUCCESS = 125,
    DELETE_FOLDER_SUCCESS = 126,
    FOLDER_NOT_FOUND = 127,
    FOLDER_ALREADY_EXIST = 128,
    FILE_NOT_FOUND = 129,
    CHANGE_SUCCESS = 130,

    FILE_LOG = 131,
};

enum PERMISSION {
    READ = 1,
    WRITE = 2,
    EXECUTE = 4,
};

vector<Account> listAccount;