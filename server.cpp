#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <vector>
#include <thread>         // For std::thread
#include <netinet/in.h>
#include <sys/socket.h>
#include "Trie.hpp"
#include "execution.hpp"

#define PORT 8080
#define BUFFER_SIZE 1024

void handleClient(int newSocket, DB::PersistentTrie* db) {
    char buffer[BUFFER_SIZE];

    DB::Users* currentUser = nullptr;

    while (currentUser == nullptr)
    {
        std::string response {"Give username and password\n"};
        send(newSocket, response.c_str(), response.length(), 0);
        memset(buffer, 0, BUFFER_SIZE);
        int valread = read(newSocket, buffer, BUFFER_SIZE);
        if (valread <= 0) break;
        std::string input(buffer);

        response = DB::execute(input, db, currentUser) ;
        send(newSocket, response.c_str(), response.length(), 0);

        if (response == DB::EXIT_RESPONSE) {
            break;
        }

        // std::cout << "reached... here\n";

    }
    


    while (true) {
        memset(buffer, 0, BUFFER_SIZE);
        int valread = read(newSocket, buffer, BUFFER_SIZE);
        if (valread <= 0) break;
        std::string input(buffer);
       
        std::string response{ DB::execute(input, db, currentUser) };
        send(newSocket, response.c_str(), response.length(), 0);

        if (response == DB::EXIT_RESPONSE) {
            break;
        }
    }
    close(newSocket);
    std::cout << "Client disconnected\n";
}

int main() {

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
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server listening on port " << PORT << "...\n";

    // Main loop: Accept clients and spawn a thread for each one.
    while (true) {

        DB::PersistentTrie *db = nullptr;

        newSocket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
        if (newSocket < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        std::cout << "Client connected\n";

        
    
        std::thread clientThread(handleClient, newSocket, db);

        clientThread.detach();
    }

    close(server_fd);
    return 0;
}
