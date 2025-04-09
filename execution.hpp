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
    const std::string EXIT_RESPONSE ( "Exited Successfully\n"s );

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
    std::string showAllCommands(){
        return std::string {"1) exit\n2) get <key>\n3) del <key>\n4) exists <key>\n5) set <key> <value>\n6) commit\n7) restore <version_id>\n8) help\n"};
    }

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

    std::string tooManyArgumentsMessage(){
        std::string{ "Too many arguments provided. Try \"help\" to know syntax\n" };
    }
    
    std::string tooFewArgumentsMessage(){
        std::cout << "Too few arguments provided. Try \"help\" to know syntax\n";
    }
    
    std::string argCountMismatch(int currSize, int expectedSize){
        if(currSize < expectedSize){
            return tooFewArgumentsMessage();
        }
        else{
            return tooManyArgumentsMessage();
        }
    }

    std::string execute(const std::string& command, PersistentTrie& trie){
        if(!isCommandValid(command)){
            return std::string{"Not a valid command\n.Try \"help\" to list all available commands\n"};
        }
        std::vector<std::string> tokens { DB::parse(command) };
        if(tokens[0] == DB::getCommandName(DB::AllCommands::exit))
        {
            return EXIT_RESPONSE;
        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::get))  
        {
            if(tokens.size() == 2){
                auto res { trie.get(tokens[1]) };
                return ((res.has_value())? res.value():"key not found"s);
            }
            else{
                return argCountMismatch(static_cast<int>(tokens.size()), 2);
            }
        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::del))
        {
            if(tokens.size() == 2){
                if(trie.exists(tokens[1])){
                    trie.remove(tokens[1]);
                    return std::string{"Key: " + tokens[1] + " removed successfully\n"};
                }
                else{
                    return std::string { "key not found\n" };
                }
            }
            else{
                return argCountMismatch(static_cast<int>(tokens.size()), 2);
            }
        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::exists))
        {
            if(tokens.size() == 2){
                return (trie.exists(tokens[1])? "true\n"s:"false\n"s);
            }
            else{
                return argCountMismatch(static_cast<int>(tokens.size()), 2);
            }   
        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::set))
        {
            if(tokens.size() == 3){
                trie.insert(tokens[1], tokens[2]);
                return std::string{"Done\n"};
            }
            else{
                return argCountMismatch(static_cast<int>(tokens.size()), 3);
            }
        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::help)){
            return showAllCommands();
        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::commit)){
            int versionId = trie.commit();
            return std::string{ "Version: "s + to_string(versionId) + " saved\n"s };
        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::restore)){
            if(tokens.size() == 2){
                int versionId{};
                try{
                    versionId = stoi(tokens[1]);
                }
                catch(...){
                    return std::string{ "Not a valid version\n"s };
                }   
                bool response { trie.restore(versionId) };
                if(response){
                    return std::string{ "Restored version "s + to_string(versionId) + '\n'};
                }
                else{
                    return "Unable to restore this version\n"s;
                }
            }
            else{
                return argCountMismatch(static_cast<int>(tokens.size()), 2);
            }
        }
    }
};



