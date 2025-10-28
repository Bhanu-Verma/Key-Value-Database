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
#include <filesystem>
#include <fstream>
#include "json.hpp"
namespace fs = std::filesystem;
using json = nlohmann::json;


namespace DB{
   inline std::mutex userMutex;
   inline std::mutex concurrentMutex;
   

   // write an initDb() here ... This function will basically load users.json and and fill the allUsers with their 
   // userName and passward here ... 



   const std::string allDatabaseFileName ="Server-Data/allDatabase.json";


   class Users
   {
   private:
   
      std::string m_password{};
      // allDatabases name
      std::set<std::string> allDatabases;
      // active_databases
      std::unordered_map<std::string, std::shared_ptr<DB::PersistentTrie>> activeDatabases;

   public:

      std::string m_name{};


      Users(std::string name, std::string password)
      : m_name{name}
      , m_password{password}
      {          
         

         std::ifstream inFile(allDatabaseFileName);
         if (!inFile) {
            std::cerr << "Failed to open database file: " << allDatabaseFileName << '\n';
            return;
         }
   
         json j;
         inFile >> j;
   
         if (j.contains(name) && j[name].is_array()) {
            for (const auto& db : j[name]) {
               allDatabases.insert(db.get<std::string>());
            }
         }

      }

      bool authenticatePassword(const std::string& password){
         return m_password == password;
      }

      std::string listDatabases(){
         std::string response{};
         for(auto db : allDatabases){
            response += db + "\r\n";
         }
         if(response==""){
            response = "No databases found.\r\n";
         }
         return response;
      }
      
      ~Users() = default;

      std::shared_ptr<DB::PersistentTrie> getDatabase(const std::string& dbName) {
         auto it = activeDatabases.find(dbName);

         if (it != activeDatabases.end()) {
            return it->second;
         }

         if(allDatabases.find(dbName) != allDatabases.end()){
            activeDatabases.emplace(dbName, std::make_shared<DB::PersistentTrie>(dbName,m_name));
            auto it = activeDatabases.find(dbName);
            std::string fileName { "./Server-Data/Data/" + m_name + '_' + dbName + ".db" };
            it->second->loadTrieFromFile(fileName);
            return it->second;
         }

         return nullptr;
      }
      

      std::string createDatabase(const std::string& dbName) {

         if(allDatabases.find(dbName) != allDatabases.end()) return std::string {"Database already exists\r\n"};

         activeDatabases.emplace(dbName, std::make_shared<DB::PersistentTrie>(dbName, m_name));
         allDatabases.insert(dbName);

         std::ifstream inFile(allDatabaseFileName);
         json j;

         if (inFile) {
            inFile >> j;
            inFile.close();
         }

         if (!j.contains(m_name) || !j[m_name].is_array()) {
            j[m_name] = json::array();
         }

         j[m_name].push_back(dbName);

         std::ofstream outFile(allDatabaseFileName);
         outFile << std::setw(4) << j << std::endl;
     
         return std::string {"Database created successfully\r\n"};

      }


   };

   std::unordered_map<std::string, std::shared_ptr<Users>> activeUsers;
   std::unordered_map<std::string, std::string> allUsers;
   std::map<std::string,int> concurrentCount;
   int totalUsers;
   const std::string allUsersFileName ="Server-Data/allUsers.json";



   bool registerUser(const std::string& username, const std::string& password) {
      std::lock_guard<std::mutex> lock(userMutex);

      // Check if user already exists
      if (allUsers.find(username) != allUsers.end()) return false;

      // Add to in-memory structures
      activeUsers.emplace(username, make_shared<Users>(Users(username, password)));
      allUsers.emplace(username, password);

      // Update the JSON file
      std::ifstream inFile(allUsersFileName);
      json j;

      if (inFile) {
         inFile >> j;
         inFile.close();
      }

      // Append new user
      j["Users"].push_back({
         {"userName", username},
         {"password", password}
      });

      // Update totalUsers
      j["totalUsers"] = j["Users"].size();

      // Save back to file
      std::ofstream outFile(allUsersFileName);
      outFile << std::setw(4) << j << std::endl;

      return true;
   }


   shared_ptr<Users> authentication(const std::string& username, const std::string& password) {

      if(activeUsers.find(username) == activeUsers.end() and  allUsers.find(username) != allUsers.end()){
         std::lock_guard<std::mutex> lock(userMutex);
         activeUsers.emplace(username, make_shared<Users>(username, password));
      }


      auto it = activeUsers.find(username);
      if (it != activeUsers.end() && it->second->authenticatePassword(password)) {
         std::cout << "Authentication Success: " << username << std::endl;

         std::lock_guard<std::mutex> lock(concurrentMutex);
         concurrentCount[username]++;

         return it->second;
      }

      return nullptr;
   }

   bool initServer(){
      // std::cout << "Filename: " << allUsersFileName << '\n';
      const fs::path allUsersFilePath { allUsersFileName};
      // std::cout << "Filepath: " << allUsersFilePath << '\n';
    

      std::ifstream myFileStream { allUsersFilePath.c_str() };
      json j = json::parse(myFileStream);

      // std::cout << "Extracting allUsers details from the file..\n";
      totalUsers = j["totalUsers"];
      // std::cout << "totalUsers: " << totalUsers << '\n';
      allUsers.clear();
      for(int i{0}; i < totalUsers; ++i){
         allUsers[j["Users"][i]["userName"]] = j["Users"][i]["password"];
         // std::cout << j["Users"][i]["userName"] << " -> " <<  j["Users"][i]["password"] << '\n';
      }

      // std::cout << "User Details -> \n";
      // for(auto [u, p]: allUsers){
      //    std::cout << "userName: " << u << ' ' << ", password: " << p << '\n'; 
      // }
      return true;
   }

}


#endif