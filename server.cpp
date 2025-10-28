#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <vector>
#include <thread>
#include <memory>
#include <mutex>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    #define CLOSE_SOCKET closesocket
    typedef int socklen_t;
#else
    #include <unistd.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #define CLOSE_SOCKET close
#endif

#include "Trie.hpp"
#include "execution.hpp"
#include "Users.hpp"

#define PORT 8080
#define BUFFER_SIZE 1024

void handleClient(int clientSocket) {
    char buffer[BUFFER_SIZE];

    std::shared_ptr<DB::Users> currentUser{nullptr};
    std::shared_ptr<DB::PersistentTrie> db{nullptr};

    // std::string response = "Authenticate First\r\n";
    // send(clientSocket, response.c_str(), response.length(), 0);

    while (currentUser == nullptr) {
        std::string response{};
        std::string prompt = "$ ";
        send(clientSocket, prompt.c_str(), prompt.length(), 0);

        memset(buffer, 0, BUFFER_SIZE);
        int valread = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (valread <= 0) break;

        std::string input(buffer);
        response = DB::execute(input, db, currentUser);
        send(clientSocket, response.c_str(), response.length(), 0);

        if (response == DB::EXIT_RESPONSE) break;
    }

    while (true) {
        std::string response{};
        std::string prompt = "$ ";
        send(clientSocket, prompt.c_str(), prompt.length(), 0);

        memset(buffer, 0, BUFFER_SIZE);
        int valread = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (valread <= 0) break;

        std::string input(buffer);
        response = DB::execute(input, db, currentUser);
        send(clientSocket, response.c_str(), response.length(), 0);

        if (response == DB::EXIT_RESPONSE) break;
    }

    std::lock_guard<std::mutex> lock(DB::concurrentMutex);
    DB::concurrentCount[currentUser->m_name]--;
    if (DB::concurrentCount[currentUser->m_name] == 0) {
        DB::activeUsers.erase(currentUser->m_name);
    }

    CLOSE_SOCKET(clientSocket);
    std::cout << "Client disconnected" << std::endl;
}

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return 1;
    }
#endif

    DB::initServer();

    int server_fd;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket failed");
        return 1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        CLOSE_SOCKET(server_fd);
        return 1;
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        CLOSE_SOCKET(server_fd);
        return 1;
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        CLOSE_SOCKET(server_fd);
        return 1;
    }

    std::cout << "Server listening on port " << PORT << "...\n";

    while (true) {
        int clientSocket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
        if (clientSocket < 0) {
            perror("accept failed");
            continue;
        }

        std::cout << "Client connected\n";
        std::thread clientThread(handleClient, clientSocket);
        clientThread.detach();
    }

    CLOSE_SOCKET(server_fd);

#ifdef _WIN32
    WSACleanup();
#endif

    return 0;
}
