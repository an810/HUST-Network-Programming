#include "resource.h"
#include "authentication.h"
#include "directory.h"
#include "file.h"

class Client {
private:
    int sock;
    bool isConnected;
    bool isLoggedIn;

public:
    Client() : isConnected(false), isLoggedIn(false) {
        mkdir(CLIENT_FOLDER, 0777);
    }

    bool connect(const char* ip, int port) {
        sock = socket(AF_INET, SOCK_STREAM, 0);

        sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, ip, &serverAddr.sin_addr);

        if (::connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            return false;
        }
        isConnected = true;
        return true;
    }

    bool login(const char* id, const char* pass) {
        isLoggedIn = Authentication::login(sock, id, pass);
        return isLoggedIn;
    }

    void listFiles() { Directory::listFiles(sock); }
    bool changeDir(const char* dir) { return Directory::changeDir(sock, dir); }
    bool uploadFile(const char* filename) { return FileHandler::uploadFile(sock, filename); }
    bool downloadFile(const char* filename) { return FileHandler::downloadFile(sock, filename); }
    bool createFolder(const char* name) { return Directory::createFolder(sock, name); }
    bool deleteFolder(const char* name) { return Directory::deleteFolder(sock, name); }
    bool renameFolder(const char* old, const char* newName) { return Directory::renameFolder(sock, old, newName); }

    ~Client() {
        if (isConnected) close(sock);
    }
};

int main() {
    Client client;
    if (!client.connect("127.0.0.1", 5500)) {
        cout << "Connection failed\n";
        return 1;
    }

    bool loggedIn = false;
    string command;

    while (true) {
        if (!loggedIn) {
            cout << "\n=== File Transfer System ===\n"
                 << "1. Login\n"
                 << "2. Exit\n"
                 << "Choose option (1-2): ";

            cin >> command;
            if (command == "1") {
                string id, pass;
                cout << "ID: "; cin >> id;
                cout << "Password: "; cin >> pass;
                if (client.login(id.c_str(), pass.c_str())) {
                    cout << "Login successful\n";
                    loggedIn = true;
                } else {
                    cout << "Login failed\n";
                }
            }
            else if (command == "2") break;
            else cout << "Invalid option\n";
            continue;
        }

        cout << "\n=== File Transfer System ===\n"
             << "1. List files\n"
             << "2. Change directory\n"
             << "3. Upload file\n"
             << "4. Download file\n"
             << "5. Create folder\n"
             << "6. Delete folder\n"
             << "7. Rename folder\n"
             << "8. Logout\n"
             << "9. Exit\n"
             << "Choose option (1-9): ";

        cin >> command;

        if (command == "1") client.listFiles();
        else if (command == "2") {
            string dir;
            cout << "Enter directory: ";
            cin >> dir;
            client.changeDir(dir.c_str());
        }
        else if (command == "3") {
            string filename;
            cout << "Enter filename: ";
            cin >> filename;
            if (client.uploadFile(filename.c_str())) {
                cout << "Upload successful\n";
            }
        }
        else if (command == "4") {
            string filename;
            cout << "Enter filename: ";
            cin >> filename;
            if (client.downloadFile(filename.c_str())) {
                cout << "Download successful\n";
            }
        }
        else if (command == "5") {
            string name;
            cout << "Enter folder name: ";
            cin >> name;
            if (client.createFolder(name.c_str())) {
                cout << "Folder created successfully\n";
            } else {
                cout << "Failed to create folder\n";
            }
        }
        else if (command == "6") {
            string name;
            cout << "Enter folder name: ";
            cin >> name;
            if (client.deleteFolder(name.c_str())) {
                cout << "Folder deleted successfully\n";
            } else {
                cout << "Failed to delete folder\n";
            }
        }
        else if (command == "7") {
            string oldName, newName;
            cout << "Enter current folder name: ";
            cin >> oldName;
            cout << "Enter new folder name: ";
            cin >> newName;
            if (client.renameFolder(oldName.c_str(), newName.c_str())) {
                cout << "Folder renamed successfully\n";
            } else {
                cout << "Failed to rename folder\n";
            }
        }
        else if (command == "8") loggedIn = false;
        else if (command == "9") break;
        else cout << "Invalid option\n";
    }
    return 0;
}