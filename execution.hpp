#ifndef EXECUTION_H 
#define EXECUTION_H

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
        save_database,
        maxCommands,
    };
    
    // Stores all the valid command
    constexpr std::array validCommands{"exit"sv, "get"sv, "del"sv, "exists"sv, "set"sv, "commit"sv, "restore"sv, "help"sv, "create_database"sv, "use_database"sv, "authenticate"sv, "create_user"sv, "save_database"sv};

    static_assert(std::size(validCommands) == maxCommands);
    
    // Get the string from enumerator
    inline constexpr std::string_view getCommandName(AllCommands command) {
        return validCommands[static_cast<std::size_t>(command)];
    }
    
    // Prints all available commands
    std::string showAllCommands(){
        return std::string {"1) exit\r\n2) get <key>\r\n3) del <key>\r\n4) exists <key>\r\n5) set <key> <value>\r\n6) commit\r\n7) restore <version_id>\r\n8) help\r\n9) create_database <db_name>\r\n10) use_database <db_name>\r\n11) authenticate <user_name> <password>\r\n12) create <user_name> <password>\r\n13) save_database <db_name>\r\n" };
    }

    /**
     * Checks if the first word of the command is valid
     */
    inline bool isCommandValid(const std::string command){

        size_t pos = command.find_first_of(" \n");
        std::string prefix = (pos == std::string::npos) ? command : command.substr(0, pos);

        // std::cout << "prefix " << prefix << '\n';

        for(auto& ch : prefix){
            ch = std::tolower(ch);
        }

        return std::find(DB::validCommands.begin(), DB::validCommands.end(), prefix) != DB::validCommands.end();

    }

    inline std::string tooManyArgumentsMessage(){
        return std::string{ "Too many arguments provided. Try \"help\" to know syntax\r\n" };
    }
    
    inline std::string tooFewArgumentsMessage(){
        return std::string{ "Too few arguments provided. Try \"help\" to know syntax\r\n" };
    }
    
    inline std::string argCountMismatch(int currSize, int expectedSize){
        if(currSize < expectedSize){
            return tooFewArgumentsMessage();
        }
        else{
            return tooManyArgumentsMessage();
        }
    }



    inline std::string execute(std::string& command, std::shared_ptr<DB::PersistentTrie>& triePtr, shared_ptr<Users>& currentUser){



        // just to remove trailing '\r' or '\n' characters
        while (!command.empty() && (command.back() == '\n' || command.back() == '\r')) {
            command.pop_back();
        }

        if(!isCommandValid(command)){
            return std::string{"Not a valid command\r\n.Try \"help\" to list all available commands\r\n"};
        }

        // std::cout << "yes\n";

        std::vector<std::string> tokens { DB::parse(command) };

        if(tokens[0] == DB::getCommandName(DB::AllCommands::create_user)){

            if(tokens.size() == 3){
                std::string userName = tokens[1];
                std::string passward = tokens[2];

                if(DB::registerUser(userName, passward))return std::string {"Registered Successfully\r\n"};
                else return std::string {"Registration Failed\r\n"};
            }
            else return argCountMismatch(static_cast<int>(tokens.size()), 3);


        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::authenticate)){
            if(tokens.size() == 3){
                std::string userName = tokens[1];
                std::string passward = tokens[2];

                currentUser = DB::authentication(userName, passward);
                if(currentUser == nullptr) return std::string {"Authentication Failed\r\n"};
                else return std::string {"Authentication Successful\r\n"};
            }
            else return argCountMismatch(static_cast<int>(tokens.size()), 3);

        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::save_database)){

            if(currentUser == nullptr) return std::string {"User not authenticated\r\n"};
            if(triePtr == nullptr) return std::string {"Database not selected\r\n"};

            if(tokens.size() == 2){
                std::string fileName { "./Server-Data/Data/" + currentUser->m_name + '_' + triePtr->m_name + ".db" };
                triePtr->saveTrie(fileName);
                return std::string {"Database saved successfully\r\n"};
            }
            else{
                return argCountMismatch(static_cast<int>(tokens.size()), 2);
            }
        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::use_database)){

            // delete triePtr;
            // triePtr = nullptr;

            if(tokens.size() == 2){
                auto dbOpt = currentUser->getDatabase(tokens[1]);
                if (dbOpt) {
                    triePtr = dbOpt;
                    return std::string {tokens[1] + " is active\r\n"};
                } else {
                    return std::string {"Database not found.\r\n"};
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
            if(triePtr == nullptr) return std::string {"Database not selected\r\n"};
            
            if(tokens.size() == 2){
                auto res { triePtr->get(tokens[1]) };
                return ((res.has_value())? res.value() + "\r\n" : "key not found\r\n"s);
            }
            else{
                return argCountMismatch(static_cast<int>(tokens.size()), 2);
            }
        }
        else if(tokens[0] == DB::getCommandName(DB::AllCommands::del))
        {
            if(triePtr == nullptr) return std::string {"Database not selected\r\n"};

            if(tokens.size() == 2){
                if(triePtr->exists(tokens[1])){
                    triePtr->remove(tokens[1]);
                    return std::string{"Key: " + tokens[1] + " removed successfully\r\n"};
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



#endif