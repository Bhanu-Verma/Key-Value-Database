#include <iostream>
#include <array>
#include <string>
#include <string_view>
#include <sstream>
#include <algorithm>
#include "parser.hpp"
#include "Trie.hpp"
#include <fstream>
#include "Users.hpp"

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
        create_database,
        use_database,
        authenticate,
        create_user,
        maxCommands,
    };
    
    // Stores all the valid command
    constexpr std::array validCommands{"exit"sv, "get"sv, "del"sv, "exists"sv, "set"sv, "commit"sv, "restore"sv, "help"sv, "create_database"sv, "use_database"sv, "authenticate"sv, "create_user"sv};

    static_assert(std::size(validCommands) == maxCommands);
    
    // Get the string from enumerator
    constexpr std::string_view getCommandName(AllCommands command) {
        return validCommands[static_cast<std::size_t>(command)];
    }
    
    // Prints all available commands
    std::string showAllCommands(){
        return std::string {"1) exit\n2) get <key>\n3) del <key>\n4) exists <key>\n5) set <key> <value>\n6) commit\n7) restore <version_id>\n8) help\n9) create_database <db_name>\n10) use_database <db_name>\n11) authenticate <user_name> <password>\n12) create <user_name> <password>\n" };
    }

    /**
     * Checks if the first word of the command is valid
     */
    bool isCommandValid(const std::string command){

        size_t pos = command.find_first_of(" \n");
        std::string prefix = (pos == std::string::npos) ? command : command.substr(0, pos);

        // std::cout << "prefix " << prefix << '\n';

        for(auto& ch : prefix){
            ch = std::tolower(ch);
        }

        return std::find(DB::validCommands.begin(), DB::validCommands.end(), prefix) != DB::validCommands.end();

    }

    std::string tooManyArgumentsMessage(){
        return std::string{ "Too many arguments provided. Try \"help\" to know syntax\n" };
    }
    
    std::string tooFewArgumentsMessage(){
        return std::string{ "Too few arguments provided. Try \"help\" to know syntax\n" };
    }
    
    std::string argCountMismatch(int currSize, int expectedSize){
        if(currSize < expectedSize){
            return tooFewArgumentsMessage();
        }
        else{
            return tooManyArgumentsMessage();
        }
    }



    std::string execute(std::string& command, PersistentTrie*& triePtr, Users*& currentUser){



        // just to remove trailing '\r' or '\n' characters
        while (!command.empty() && (command.back() == '\n' || command.back() == '\r')) {
            command.pop_back();
        }

        if(!isCommandValid(command)){
            return std::string{"Not a valid command\n.Try \"help\" to list all available commands\n"};
        }

        // std::cout << "yes\n";

        std::vector<std::string> tokens { DB::parse(command) };

        if(tokens[0] == DB::getCommandName(DB::AllCommands::create_user)){

            if(tokens.size() == 3){
                std::string userName = tokens[1];
                std::string passward = tokens[2];

                if(DB::registerUser(userName, passward))return std::string {"Registered Successfully\n"};
                else return std::string {"Registration Failed\n"};
            }
            else return argCountMismatch(static_cast<int>(tokens.size()), 3);


        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::authenticate)){
            if(tokens.size() == 3){
                std::string userName = tokens[1];
                std::string passward = tokens[2];

                currentUser = DB::authentication(userName, passward);
                if(currentUser == nullptr) return std::string {"Authentication Failed\n"};
                else return std::string {"Authentication Successful\n"};
            }
            else return argCountMismatch(static_cast<int>(tokens.size()), 3);

        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::use_database)){

            if(tokens.size() == 2){
                auto dbOpt = currentUser->getDatabase(tokens[1]);
                if (dbOpt) {
                    triePtr = dbOpt.get();
                    return std::string {tokens[1] + " is active\n"};
                } else {
                    return std::string {"Database not found.\n"};
                }
            }
            else{
                return argCountMismatch(static_cast<int>(tokens.size()), 2);
            }

        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::create_database)){

            if(tokens.size() == 2){
                return currentUser->createDatabase(tokens[1]);
            }
            else{
                return argCountMismatch(static_cast<int>(tokens.size()), 2);
            }

        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::exit))
        {
            return EXIT_RESPONSE;
        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::get))  
        {
            if(triePtr == nullptr) return std::string {"Database not selected\n"};
            
            if(tokens.size() == 2){
                auto res { triePtr->get(tokens[1]) };
                return ((res.has_value())? res.value() + '\n' : "key not found\n"s);
            }
            else{
                return argCountMismatch(static_cast<int>(tokens.size()), 2);
            }
        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::del))
        {
            if(triePtr == nullptr) return std::string {"Database not selected\n"};

            if(tokens.size() == 2){
                if(triePtr->exists(tokens[1])){
                    triePtr->remove(tokens[1]);
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
            if(triePtr == nullptr) return std::string {"Database not selected\n"};

            if(tokens.size() == 2){
                return (triePtr->exists(tokens[1])? "true\n"s:"false\n"s);
            }
            else{
                return argCountMismatch(static_cast<int>(tokens.size()), 2);
            }   
        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::set))
        {
            if(triePtr == nullptr) return std::string {"Database not selected\n"};

            if(tokens.size() == 3){
                triePtr->insert(tokens[1], tokens[2]);
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

            if(triePtr == nullptr) return std::string {"Database not selected\n"};

            int versionId = triePtr->commit();
            return std::string{ "Version: "s + to_string(versionId) + " saved\n"s };
        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::restore)){

            if(triePtr == nullptr) return std::string {"Database not selected\n"};

            if(tokens.size() == 2){
                int versionId{};
                try{
                    versionId = stoi(tokens[1]);
                }
                catch(...){
                    return std::string{ "Not a valid version\n"s };
                }   
                bool response { triePtr->restore(versionId) };
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
        return std::string{"Unhandled or incomplete command\n"};

    }
};



