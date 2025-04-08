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
        maxCommands,
    };
    
    // Stores all the valid command
    constexpr std::array validCommands{"exit"sv, "get"sv, "del"sv, "exists"sv, "set"sv};
    static_assert(std::size(validCommands) == maxCommands);

    // Get the string from enumerator
    constexpr std::string_view getCommandName(AllCommands command) {
        return validCommands[static_cast<std::size_t>(command)];
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


int main(){
    while (true)
    {
        std::cout << "$ ";
        std::string command{};

        // Take the input command 
        std::getline(std::cin, command);

        // Check for the validity of the command
        if(!isCommandValid(command)){
            std::cout << "Not a valid command\n";
            continue;
        }

        std::vector<std::string> tokens { DB::parse(command) };
        
        if(tokens[0] == DB::getCommandName(DB::AllCommands::exit))
        {
            break;
        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::get))  
        {

        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::del))
        {

        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::exists))
        {

        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::set))
        {

        }
        else{
            
        }
    }
    
    return 0;
}