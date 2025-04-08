#include <iostream>
#include <array>
#include <string>
#include <string_view>
#include <sstream>
#include <algorithm>
#include "parser.hpp"

namespace DB{
    using namespace std::literals;

    enum AllCommands{
        exit,
        get,
        del,
        exists,
        set,
        help,
        maxCommands,
    };
    
    // Stores all the valid command
    constexpr std::array validCommands{"exit"sv, "get"sv, "del"sv, "exists"sv, "set"sv, "help"sv};
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
        std::cout << "6) help\n";
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
        for(const auto& s: tokens){
            std::cout << s << '\n';
        }
        std::cout << '\n';
        if(tokens[0] == DB::getCommandName(DB::AllCommands::exit))
        {
            break;
        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::get))  
        {
            if(tokens.size() == 2){
                // TODO
            }
            else{
                argCountMismatch(static_cast<int>(tokens.size()), 2);
            }
        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::del))
        {
            if(tokens.size() == 2){
                // TODO
            }
            else{
                argCountMismatch(static_cast<int>(tokens.size()), 2);
            }
        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::exists))
        {
            if(tokens.size() == 2){
                // TODO
            }
            else{
                argCountMismatch(static_cast<int>(tokens.size()), 2);
            }   
        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::set))
        {
            if(tokens.size() == 3){
                // TODO
            }
            else{
                argCountMismatch(static_cast<int>(tokens.size()), 3);
            }
        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::help)){
            DB::showAllCommands();
        }
        else{

        }
    }
    
    return 0;
}