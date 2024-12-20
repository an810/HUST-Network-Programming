#include "resource.h"
#include "authentication.h"
#include "directory.h"
#include "file.h"

std::vector<Account> accounts;
std::vector<ClientInfo> clients;

void handleClientRequest(int sock, ClientInfo &client, Message &msg) {
    int bytes = recv(sock, &msg, sizeof(Message), 0);
    if (bytes <= 0) return;  // Connection closed or error

    std::cout << "Received message from client " << msg.opcode << std::endl;

    switch (msg.opcode) {
        case LOGIN:
            Authentication::handleLogin(sock, msg, accounts);
            break;
        case CHANGE_DIR:
            Directory::changeDirectory(client, msg);
            break;
        case LIST_FILES:
            Directory::listFiles(client, msg);
            break;
        case CREATE_FOLDER:
            Directory::createDirectory(client, msg);
            break;
        case DELETE_FOLDER:
            Directory::deleteDirectory(client, msg);
            break;
        case RENAME_FOLDER:
            Directory::renameDirectory(client, msg);
            break;
        case UPLOAD:
        case DOWNLOAD:
        case DATA_UP:
        case DATA_DOWN:
        case CREATE_FILE:
        case DELETE_FILE:
        case RENAME_FILE:
        case SEARCH_FILE:
            FileHandler::handleFileTransfer(sock, msg, client);
            break;
    }
    std::cout << "Sending response to client " << msg.opcode << std::endl;
    send(sock, &msg, sizeof(Message), 0);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }

    mkdir(SERVER_FOLDER, 0777);
    Authentication::loadAccounts(accounts);

    int serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(atoi(argv[1]));

    if (bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        return 1;
    }

    if (listen(serverSock, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        return 1;
    }

    std::cout << "Server started on port " << argv[1] << std::endl;

    fd_set readfds;
    int maxSock = serverSock;

    while (true) {
        FD_ZERO(&readfds);
        FD_SET(serverSock, &readfds);

        // Add all client sockets to the fd_set
        for (auto &client : clients) {
            FD_SET(client.socket, &readfds);
            maxSock = std::max(maxSock, client.socket);
        }

        // Use select() to monitor multiple sockets
        int activity = select(maxSock + 1, &readfds, nullptr, nullptr, nullptr);
        if (activity < 0) {
            perror("Select error");
            break;
        }

        // Check if there is a new incoming connection
        if (FD_ISSET(serverSock, &readfds)) {
            struct sockaddr_in clientAddr;
            socklen_t clientLen = sizeof(clientAddr);
            int clientSock = accept(serverSock, (struct sockaddr*)&clientAddr, &clientLen);
            if (clientSock < 0) {
                perror("Accept failed");
                continue;
            }

            std::cout << "New client connected" << std::endl;

            // Add new client to the clients list
            ClientInfo client = {clientSock, nullptr, 0, 0};
            strcpy(client.currentDir, SERVER_FOLDER);
            clients.push_back(client);

            // Add the new client socket to the fd_set
            FD_SET(clientSock, &readfds);
        }

        // Handle all the active client sockets
        for (auto it = clients.begin(); it != clients.end(); ) {
            if (FD_ISSET(it->socket, &readfds)) {
                Message msg;
                handleClientRequest(it->socket, *it, msg);
            }
            ++it;
        }
    }

    // Cleanup
    close(serverSock);
    return 0;
}
