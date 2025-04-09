#include <iostream>
#include "parser.hpp"
#include "execution.hpp"

int main(){
    DB::PersistentTrie db{};
    while (true)
    {
        string command{};
        std::getline(std::cin >> std::ws, command);
        std::string response { DB::execute(command, db) };
        if(response == DB::EXIT_RESPONSE){
            break;
        }
        std::cout << response << '\n';
    }
    
    return 0;
}