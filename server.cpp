#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <vector>
#include <netinet/in.h>
#include <sys/socket.h>
#include "Trie.hpp"
#include "execution.hpp"

#define PORT 8080
#define BUFFER_SIZE 1024

void handleClient(int newSocket, DB::PersistentTrie& db) {
    char buffer[BUFFER_SIZE];
    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int valread = read(newSocket, buffer, BUFFER_SIZE);
        if (valread <= 0) break;

        std::string input(buffer);
        std::string response{ DB::execute(input) };
        send(newSocket, response.c_str(), response.length(), 0);
        if(response == DB::EXIT_RESPONSE){
            break;
        }
    }

    close(newSocket);
    std::cout << "Client disconnected\n";
}

int main() {
    TrieDatabase db;

    int server_fd, newSocket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket file descriptor
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attach socket to the port
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 3);

    std::cout << "Server listening on port " << PORT << "...\n";

    while (true) {
        newSocket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (newSocket < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        std::cout << "Client connected\n";
        handle_client(newSocket, db);
    }

    close(server_fd);
    return 0;
}
