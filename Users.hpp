#ifndef USERS_H
#define USERS_H

#include "Trie.hpp"
#include <string>
#include <unordered_map>
#include <optional>
#include <functional>
#include <iostream>
#include <mutex>
#include <set>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;
using json = nlohmann::json;
namespace DB{
   inline std::mutex userMutex;
   

   // write an initDb() here ... This function will basically load users.json and and fill the allUsers with their 
   // userName and passward here ... 





   class Users
   {
   private:
   
   std::string m_name{};
   std::string m_password{};
   // allDatabases name
   std::set<std::string> allDatabases;
   // active_databases
   std::unordered_map<std::string, std::shared_ptr<DB::PersistentTrie>> activeDatabases;

   public:
      Users(std::string name, std::string password)
      : m_name{name}
      , m_password{password}
      {          
                 
         //  apply an encrypter here
         //  jab bhi ek naya user bana rahe --> load all all the databases allDatabases form the dbs.json file

      }

      bool authenticatePassword(const std::string& password){
         return m_password == password;
         //  apply a decryptor here
      }
      
      ~Users() = default;

      std::shared_ptr<DB::PersistentTrie> getDatabase(const std::string& dbName) {
         auto it = activeDatabases.find(dbName);

         if (it != activeDatabases.end()) {
            return it->second;
         }

         if(allDatabases.find(dbName) != allDatabases.end()){
            activeDatabases.emplace(dbName, std::make_shared<DB::PersistentTrie>());
            auto it = activeDatabases.find(dbName);
            std::string fileName { m_name + '_' + dbName + ".db" };
            // (it->second).get().loadTrieFromFile(fileName);
            return it->second;
         }

         return nullptr;
      }
      

      std::string createDatabase(const std::string& dbName) {

         if(allDatabases.find(dbName) != allDatabases.end()) return std::string {"Database already exists\n"};

         activeDatabases.emplace(dbName, std::make_shared<DB::PersistentTrie>());
         allDatabases.insert(dbName);

         // append to the json file also
         
         return std::string {"Database created successfully\n"};
      }


   };

   std::unordered_map<std::string, Users> activeUsers;
   std::unordered_map<std::string, std::string> allUsers;
   int totalUsers;
   const std::string allUsersFileName = "C:\\Users\\ramudevasani\\Desktop\\Projects\\Key-Value-Database\\Server-Data\\allUsers.json";


// init function allUser.json -> 

   bool registerUser(const std::string& username, const std::string& password) {
      std::lock_guard<std::mutex> lock(userMutex);

      if(allUsers.find(username) != allUsers.end())return false;
      activeUsers.emplace(username, Users(username, password));
      allUsers.emplace(username,password);

      // append to the json file also

      return true;
   }

   Users* authentication(const std::string& username, const std::string& password) {

      if(activeUsers.find(username) == activeUsers.end() and  allUsers.find(username) != allUsers.end()){
         std::lock_guard<std::mutex> lock(userMutex);
         activeUsers.emplace(username, Users(username, password));
      }

      auto it = activeUsers.find(username);
      if (it != activeUsers.end() && it->second.authenticatePassword(password)) {
         std::cout << "authentication success in Users\n";
         return &(it->second);
      }

      return nullptr;
   }

   bool initServer(){
      std::cout << "Filename: " << allUsersFileName << '\n';
      const fs::path allUsersFilePath { allUsersFileName};
      std::cout << "Filepath: " << allUsersFilePath << '\n';
      if(allUsersFilePath.extension() != "json"){
         std::cout << "Filename doesn't exist.\n";
         return false;
      }
      std::ifstream myFileStream { allUsersFilePath.c_str() };
      json j = json::parse(myFileStream);

      std::cout << "Extracting allUsers details from the file..\n";
      totalUsers = j["totalUsers"];
      std::cout << "totalUsers: " << totalUsers << '\n';
      allUsers.clear();
      for(int i{0}; i < totalUsers; ++i){
         allUsers[j["Users"]["userName"]] = allUsers[j["Users"]["password"]];
      }

      std::cout << "User Details -> \n";
      for(auto [u, p]: allUsers){
         std::cout << "userName: " << u << ' ' << ", password: " << p << '\n'; 
      }
      return true;
   }

}


#endif