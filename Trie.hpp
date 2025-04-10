#ifndef TRIE_H
#define TRIE_H

#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <optional>
#include <mutex>
#include <random>
#include <cassert>
#include <shared_mutex>
#include <fstream>
#include <stack>
#include <filesystem>
using namespace std;

namespace DB{
    struct TrieNode {
        int id{-1};
        bool isEndOfWord{};
        std::unordered_map<char, shared_ptr<TrieNode>> children{};

        TrieNode(bool isEnd = false, int _id=-1)
        : isEndOfWord{ isEnd }
        , id{ _id }
        {}
    };

    class PersistentTrie {
    private: 
        std::vector<std::string> m_storage;
        vector<shared_ptr<TrieNode>> versions;
        shared_ptr<TrieNode> currentRoot;
        mutable std::shared_mutex sh_mtx;
        std::string filename{"SerialTrie.bin"};


        shared_ptr<TrieNode> insert(shared_ptr<TrieNode> node, const string &word, int index, const std::string& val) {
            if (!node) node = make_shared<TrieNode>();
            shared_ptr<TrieNode> newNode = make_shared<TrieNode>(*node); 

            if (index == word.size()) {
                // put the word in storage 
                m_storage.push_back(val);
                newNode->isEndOfWord = true;
                newNode->id = m_storage.size() - 1;
                return newNode;
            }

            char ch = word[index];
            newNode->children[ch] = insert(node->children[ch], word, index + 1, val);
            return newNode;
        }

        bool exists(shared_ptr<TrieNode> node, const string &word, int index) const {
            if (!node) return false;
            if (index == word.size()) return node->isEndOfWord;
            char ch = word[index];
            return exists(node->children[ch], word, index + 1);
        }

        shared_ptr<TrieNode> remove(shared_ptr<TrieNode> node, const string &key, int index) {
            if (!node) return nullptr;       
            shared_ptr<TrieNode> newNode = make_shared<TrieNode>(*node);

            if (index == key.size()) {
                if (!newNode->isEndOfWord) return newNode; // key doesn't exist
                newNode->isEndOfWord = false;
                newNode->id = -1;
                if(newNode->children.empty()){
                    return nullptr;
                }
                return newNode;
            }
        
            char ch = key[index];
            auto removed = remove(newNode->children[ch], key, index + 1);
            if(removed){
                newNode->children[ch] = removed;
            }
            else{
                if(newNode->children.find(ch) != newNode->children.end())
                    newNode->children.erase(ch);
                if(newNode->children.empty() && (newNode->id)==-1){
                    return nullptr;
                }
            }
            return newNode;
        }

        void loadStorage(std::string& s){
            m_storage.clear();
            for(int i{0}; i < s.size(); ++i){
                std::string temp{};
                while(i < s.size() && s[i] != '>'){
                    temp.push_back(s[i]);
                    ++i;
                }
                if(!(temp.empty())){
                    m_storage.push_back(temp);
                }
            }
        }
        

    public:
        PersistentTrie()
        : currentRoot{make_shared<TrieNode>()}
        {
            if (std::filesystem::exists(filename)) {
                std::ifstream fin(filename);
                string s{};
                fin >> s;
                std::cout << "vector " << s << std::endl;
                loadStorage(s);
                fin >> s;
                std::cout << "Trie " << s << std::endl;
                currentRoot = deserializeTrie(s);
            }
        }

        void insert(const string &word, const std::string& val) {
            std::unique_lock<std::shared_mutex> lock(sh_mtx);   // Unique lock for writers
            currentRoot = insert(currentRoot, word, 0, val);
        }

        int commit() {
            std::unique_lock<std::shared_mutex> lock(sh_mtx);
            versions.push_back(currentRoot);
            return versions.size() - 1; // return version index
        }

        bool exists(const string &key) const {
            std::shared_lock<std::shared_mutex> lock(sh_mtx);
            return exists(currentRoot, key, 0);
        }

        int versionCount() const {
            std::shared_lock<std::shared_mutex> lock(sh_mtx);
            return versions.size();
        }

        void remove(const string &key) {
            std::unique_lock<std::shared_mutex> lock(sh_mtx);
            currentRoot = remove(currentRoot, key, 0);
        }
        
        std::optional<std::string> get(std::string_view key) const {
            std::shared_lock<std::shared_mutex> lock(sh_mtx);
            shared_ptr<TrieNode> node = currentRoot;
            for (char ch : key) {
                if (!node || node->children.find(ch) == node->children.end()) {
                    return std::nullopt;
                }
                node = node->children.at(ch);
            }
            if (node && node->isEndOfWord && (node->id)!=-1) {
                return m_storage[node->id];
            }
            return std::nullopt;
        }

        bool restore(int version) {
            std::unique_lock<std::shared_mutex> lock(sh_mtx);   
            if (version < 0 || version >= versions.size()) {
                return false;
            }
        
            // Save current state as a new version before restoring
            versions.push_back(currentRoot);
        
            // Set current root to a copy of the selected version's root
            currentRoot = versions[version];
            return true;
        }

        void serializeTrie(shared_ptr<TrieNode> node, string& s){
            if(node->isEndOfWord){
                s.push_back('[');
                s += to_string(node->id);
                s.push_back(']');
            }
            for(auto child : node->children){
                s.push_back(child.first);
                serializeTrie(child.second, s);
            }
            s.push_back('>');
        }

        shared_ptr<TrieNode> deserializeTrie(string& s){
            stack<shared_ptr<TrieNode>> st;
            st.push(make_shared<TrieNode>());
            std::size_t length { static_cast<std::size_t>(s.size()) };
            for(std::size_t i { 0 }; i < length; ++i){
                assert(s[i] != ']' && "Invalid serialized trie is passed");
                if(s[i] == '['){
                    std::string valueId { };
                    ++i;
                    while(i < length && s[i] != ']'){
                        valueId.push_back(s[i]);
                        ++i;
                    }
                    assert(i < length && "Invalid serialized trie is passed");
                    st.top()->id = std::stoi(valueId);
                    st.top()->isEndOfWord = true;
                }else if(s[i] == '>'){
                    assert(!(st.empty()) && "Invalid serialized trie is passed");
                    st.pop();
                }else{
                    shared_ptr<TrieNode> currentNode { make_shared<TrieNode>() };
                    st.top()->children[s[i]] = currentNode;
                    st.push(currentNode);
                }
            }
            assert(st.size() == 1 && "Invalid serialized trie is passed");
            return st.top(); 
        }

        void saveCurrentState(){
            std::ofstream fout("SerialTrie.bin");
            for(auto& i : m_storage){
                fout << i;
                fout << '>';
            }
            fout << '\n';
            fout << serialize();
            fout.close();
        }
 
        string serialize(){
            string s{};
            serializeTrie(currentRoot,s);
            s.pop_back();
            return s;
        }
        
    };
}


#endif