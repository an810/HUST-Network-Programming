#include "resource.h"
#include "authentication.h"
#include "directory.h"
#include "file.h"

std::vector<Account> accounts;
std::vector<ClientInfo> clients;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* clientHandler(void* arg) {
    int sock = *(int*)arg;
    ClientInfo client = {sock, nullptr, 0, 0};
    strcpy(client.currentDir, SERVER_FOLDER);

    pthread_mutex_lock(&mutex);
    clients.push_back(client);
    pthread_mutex_unlock(&mutex);

    Message msg;
    while (true) {
        int bytes = recv(sock, &msg, sizeof(Message), 0);
        if (bytes <= 0) break;

        switch (msg.opcode) {
            case LOGIN:
                Authentication::handleLogin(sock, msg, accounts);
                break;
            case CHANGE_DIR:
                Directory::changeDirectory(client, msg.payload);
                break;
            case LIST_FILES:
                Directory::listFiles(client.currentDir, msg.payload);
                break;
            case CREATE_FOLDER:
                Directory::createDirectory(msg);
                break;
            case DELETE_FOLDER:
                Directory::deleteDirectory(msg);
                break;
            case RENAME_FOLDER:
                Directory::renameDirectory(msg);
                break;
            case UPLOAD:
            case DOWNLOAD:
            case DATA_UP:
            case DATA_DOWN:
                FileHandler::handleFileTransfer(sock, msg, clients);
                break;
        }

        send(sock, &msg, sizeof(Message), 0);
    }

    close(sock);
    return nullptr;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }

    mkdir(SERVER_FOLDER, 0777);
    Authentication::loadAccounts(accounts);

    int serverSock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(atoi(argv[1]));

    bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSock, MAX_CLIENTS);

    std::cout << "Server started on port " << argv[1] << std::endl;

    while (true) {
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);

        int clientSock = accept(serverSock, (struct sockaddr*)&clientAddr, &clientLen);

        pthread_t thread;
        pthread_create(&thread, nullptr, clientHandler, &clientSock);
        pthread_detach(thread);
    }

    return 0;
}