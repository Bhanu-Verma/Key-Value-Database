#ifndef USERS_H
#define USERS_H


#include "Trie.hpp"
#include <string>
#include <unordered_map>
#include <optional>
#include <functional>
#include <iostream>
#include <mutex>


namespace DB{

   inline std::mutex userMutex;


   class Users
   {
   private:
   
   std::string m_name{};
   std::string m_password{};
   // std::set<DB::PersistentTrie> m_databases{};
   std::unordered_map<std::string, std::shared_ptr<DB::PersistentTrie>> m_databases;


   public:

      Users(std::string name, std::string password)
      : m_name{name}
      , m_password{password}
      {         
         //  apply an encrypter here
      }

      bool authenticatePassword(const std::string& password){
         return m_password == password;
         //  apply a decryptor here
      }
      
      ~Users() = default;

      std::shared_ptr<DB::PersistentTrie> getDatabase(const std::string& dbName) {
         auto it = m_databases.find(dbName);
         if (it != m_databases.end()) {
             return it->second;
         }
         return nullptr;
      }
      

      std::string createDatabase(const std::string& dbName) {
         if(m_databases.find(dbName) != m_databases.end()){
            return std::string {"Database already exists\n"};
         }
         else{
            m_databases.emplace(dbName, std::make_shared<DB::PersistentTrie>());
            return std::string {"Database created successfully\n"};
         }
      }
      

   };
   
   
   std::unordered_map<std::string, Users> currentUsers;



   bool registerUser(const std::string& username, const std::string& password) {
      std::lock_guard<std::mutex> lock(userMutex);
      if (currentUsers.find(username) != currentUsers.end()) return false;
      currentUsers.emplace(username, Users(username, password));
      return true;
   }


   Users* authentication(const std::string& username, const std::string& password) {
      auto it = currentUsers.find(username);
      if (it != currentUsers.end() && it->second.authenticatePassword(password)) {
         std::cout << "authentication success in Users\n";
         return &(it->second);
      }
      return nullptr;
   }
   
}


#endif