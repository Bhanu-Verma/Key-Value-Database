#include <iostream>
#include <array>
#include <string>
#include <string_view>
#include <sstream>
#include <algorithm>
#include "parser.hpp"
#include "Trie.hpp"

namespace DB{
    using namespace std::literals;

    enum AllCommands{
        exit,
        get,
        del,
        exists,
        set,
        commit,
        restore,
        help,
        maxCommands,
    };
    
    // Stores all the valid command
    constexpr std::array validCommands{"exit"sv, "get"sv, "del"sv, "exists"sv, "set"sv, "commit"sv, "restore"sv, "help"sv};
    static_assert(std::size(validCommands) == maxCommands);

    // Get the string from enumerator
    constexpr std::string_view getCommandName(AllCommands command) {
        return validCommands[static_cast<std::size_t>(command)];
    }

    // Prints all available commands
    void showAllCommands(){
        std::cout << "1) exit\n";
        std::cout << "2) get <key>\n";
        std::cout << "3) del <key>\n";
        std::cout << "4) exists <key>\n";
        std::cout << "5) set <key> <value>\n";
        std::cout << "6) commit\n";
        std::cout << "7) restore <version_id>\n";
        std::cout << "8) help\n";
    }
};

/**
 * Checks if the first word of the command is valid
 */
bool isCommandValid(const std::string& command){
    size_t pos = command.find_first_of(" \n");
    std::string prefix = (pos == std::string::npos) ? command : command.substr(0, pos);
    std::transform(prefix.begin(), prefix.end(), prefix.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return std::find(DB::validCommands.begin(), DB::validCommands.end(), prefix) != DB::validCommands.end();
}

inline void tooManyArgumentsMessage(){
    std::cout << "Too many arguments provided. Try \"help\" to know syntax\n";
}

inline void tooFewArgumentsMessage(){
    std::cout << "Too few arguments provided. Try \"help\" to know syntax\n";
}

inline void argCountMismatch(int currSize, int expectedSize){
    if(currSize < expectedSize){
        tooFewArgumentsMessage();
    }
    else{
        tooManyArgumentsMessage();
    }
}

int main(){

    DB::PersistentTrie trie{};

    while (true)
    {
        std::cout << "$ ";
        std::string command{};

        // Take the input command 
        std::getline(std::cin >> std::ws, command);  

        // Check for the validity of the command
        if(!isCommandValid(command)){
            std::cout << "Not a valid command\n";
            std::cout << "Try \"help\" to list all available commands\n";
            continue;
        }

        std::vector<std::string> tokens { DB::parse(command) };
        if(tokens[0] == DB::getCommandName(DB::AllCommands::exit))
        {
            break;
        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::get))  
        {
            if(tokens.size() == 2){
                auto res { trie.get(tokens[1]) };
                if(res.has_value()){
                    std::cout << res.value() << '\n';
                }
                else{
                    std::cout << "key not found\n";
                }
            }
            else{
                argCountMismatch(static_cast<int>(tokens.size()), 2);
            }
        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::del))
        {
            if(tokens.size() == 2){
                if(trie.exists(tokens[1])){
                    trie.remove(tokens[1]);
                    std::cout << "Key: " << tokens[1] << " removed successfully\n";
                }
                else{
                    std::cout << "key not found\n";
                }
            }
            else{
                argCountMismatch(static_cast<int>(tokens.size()), 2);
            }
        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::exists))
        {
            if(tokens.size() == 2){
                std::cout << std::boolalpha;
                std::cout << trie.exists(tokens[1]) << '\n';
            }
            else{
                argCountMismatch(static_cast<int>(tokens.size()), 2);
            }   
        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::set))
        {
            if(tokens.size() == 3){
                trie.insert(tokens[1], tokens[2]);
                std::cout << "One value inserted successfully\n";
            }
            else{
                argCountMismatch(static_cast<int>(tokens.size()), 3);
            }
        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::help)){
            DB::showAllCommands();
        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::commit)){
            int versionId = trie.commit();
            std::cout << "Version: " << versionId << " saved\n";
        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::restore)){
            if(tokens.size() == 2){
                int versionId{};
                try{
                    versionId = stoi(tokens[1]);
                }
                catch(...){
                    std::cout << "Not a valid version\n";
                }   
                bool response { trie.restore(versionId) };
                if(response){
                    std::cout << "Restored version " << versionId << '\n';
                }
                else{
                    std::cout << "Unable to restore this version\n";
                }
            }
            else{
                argCountMismatch(static_cast<int>(tokens.size()), 2);
            }
        }
    }
    
    

    return 0;
}