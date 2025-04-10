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
#include <set>
#include <map>
using namespace std;

namespace DB
{
    struct TrieNode
    {
        int m_id{}, index{-1};
        bool isEndOfWord{};
        std::unordered_map<char, shared_ptr<TrieNode>> children{};

        TrieNode(int id, bool isEnd = false, int _index = -1)
            : m_id{id}, isEndOfWord{isEnd}, index{_index}
        {
        }
    };

    // ok

    class PersistentTrie
    {
    private:
        int totalNodes{};
        std::vector<std::string> m_storage;
        vector<shared_ptr<TrieNode>> versions;
        shared_ptr<TrieNode> currentRoot;
        mutable std::shared_mutex sh_mtx;

        shared_ptr<TrieNode> insert(shared_ptr<TrieNode> node, const string &word, int index, const std::string &val)
        {
            if (!node)
            {
                ++totalNodes;
                node = make_shared<TrieNode>(totalNodes);
            }
            shared_ptr<TrieNode> newNode = make_shared<TrieNode>(*node);
            ++totalNodes;
            newNode->m_id = totalNodes;

            if (index == word.size())
            {
                // put the word in storage
                m_storage.push_back(val);
                newNode->isEndOfWord = true;
                newNode->index = m_storage.size() - 1;
                return newNode;
            }
            if(word=="ct" && index==1){
                std::cout << "printing map:\n";
                for(auto p: newNode->children){
                    std::cout << p.first << ' ' << p.second << '\n';
                }
            }
            char ch = word[index];
            auto inserted = insert(node->children[ch], word, index + 1, val);
            assert(inserted && "insert returned null");
            newNode->children[ch] = inserted;
            return newNode;
        }

        bool exists(shared_ptr<TrieNode> node, const string &word, int index) const
        {
            if (!node)
                return false;
            if (index == word.size())
                return node->isEndOfWord;
            char ch = word[index];
            if(node->children.find(ch) == node->children.end()){
                return false;
            }
            return exists(node->children[ch], word, index + 1);
        }

        shared_ptr<TrieNode> remove(shared_ptr<TrieNode> node, const string &key, int index)
        {
            if (!node)
                return nullptr;
            shared_ptr<TrieNode> newNode = make_shared<TrieNode>(*node);
            ++totalNodes;
            newNode->m_id = totalNodes;

            if (index == key.size())
            {
                if (!(newNode->isEndOfWord))
                    return newNode; // key doesn't exist
                newNode->isEndOfWord = false;
                newNode->index = -1;
                if (newNode->children.empty())
                {
                    return nullptr;
                }
                return newNode;
            }

            char ch = key[index];
            if(newNode->children.find(ch) == newNode->children.end()){
                std::cout << "char " << ch << " not present in children map\n";
                return newNode;
            }
            auto removed = remove(newNode->children[ch], key, index + 1);
            if (removed)
            {
                newNode->children[ch] = removed;
            }
            else
            {
                // if (newNode->children.find(ch) != newNode->children.end())
                newNode->children.erase(ch);
                if (index!=0 && newNode->children.empty() && (newNode->index) == -1)
                {
                    return nullptr;
                }
            }
            return newNode;
        }

        void loadStorage(std::string &s)
        {
            m_storage.clear();
            for (int i{0}; i < s.size(); ++i)
            {
                std::string temp{};
                while (i < s.size() && s[i] != '>')
                {
                    temp.push_back(s[i]);
                    ++i;
                }
                if (!(temp.empty()))
                {
                    m_storage.push_back(temp);
                }
            }
        }

        void loadNodePointers(std::string& s,std::map<int, shared_ptr<TrieNode>>& nodePointers){
            totalNodes = -1;
            std::size_t length{static_cast<std::size_t>(s.size())};
            nodePointers.clear();
            for(std::size_t i{0}; i < length; ++i){
                std::string nodeId{},nodeValue{};
                while(i < length && s[i] != '['){
                    nodeId.push_back(s[i]);
                    ++i;
                }
                ++i;
                while(i < length && s[i] != ']'){
                    nodeValue.push_back(s[i]);
                    ++i;
                }
                ++totalNodes;
                nodePointers[stoi(nodeId)] = make_shared<TrieNode>(totalNodes);
                if(stoi(nodeValue) != -1){
                    nodePointers[stoi(nodeId)]->isEndOfWord = true;
                    nodePointers[stoi(nodeId)]->index = stoi(nodeValue);
                }
            }
        }

    public:
        PersistentTrie()
            : totalNodes{0}, currentRoot{make_shared<TrieNode>(totalNodes)}
        {}

        void loadTrieFromFile(const std::string& filename){
            assert(std::filesystem::exists(filename) && "file doesn't exists");
            std::ifstream fin(filename);
            string s_storage{}, s_nodePointers{}, s_trie{};
            getline(fin, s_storage);
            assert(!(fin.eof()) && "Invalid Trie in file");
            getline(fin, s_nodePointers);
            assert(!(fin.eof()) && "Invalid Trie in file");
            getline(fin, s_trie);
            assert((fin.eof()) && "Invalid Trie in file");
            // std::cout << "vector: " << s << std::endl;
            loadStorage(s_storage);
            // fin >> s;
            // std::cout << "Nodes: " << s << std::endl;
            std::map<int, shared_ptr<TrieNode>> nodePointers;
            loadNodePointers(s_nodePointers, nodePointers);
            // fin >> s;
            // std::cout << "Trie: " << s << std::endl;
            versions = deserializeTrie(s_trie, nodePointers);
            if(!versions.empty()){
                currentRoot = versions.back();
                versions.pop_back();
            }
        }

        void insert(const string &word, const std::string &val)
        {
            std::unique_lock<std::shared_mutex> lock(sh_mtx); // Unique lock for writers
            currentRoot = insert(currentRoot, word, 0, val);
        }

        int commit()
        {
            std::unique_lock<std::shared_mutex> lock(sh_mtx);
            versions.push_back(currentRoot);
            return versions.size() - 1; // return version index
        }

        bool exists(const string &key) const
        {
            std::shared_lock<std::shared_mutex> lock(sh_mtx);
            return exists(currentRoot, key, 0);
        }

        int versionCount() const
        {
            std::shared_lock<std::shared_mutex> lock(sh_mtx);
            return versions.size();
        }

        void remove(const string &key)
        {
            if(!exists(key)){
                return;
            }
            std::unique_lock<std::shared_mutex> lock(sh_mtx);
            currentRoot = remove(currentRoot, key, 0);
        }

        std::optional<std::string> get(const std::string& key) const
        {
            if(!exists(key)){
                return std::nullopt;
            }
            std::shared_lock<std::shared_mutex> lock(sh_mtx);
            shared_ptr<TrieNode> node = currentRoot;
            for (char ch : key)
            {
                if (!node || node->children.find(ch) == node->children.end())
                {
                    std::cout << "here..\n";
                    return std::nullopt;
                }
                node = node->children.at(ch);
            }
            if (node && node->isEndOfWord && (node->index) != -1)
            {
                return m_storage[node->index];
            }
            std::cout << "here..\n";
            return std::nullopt;
        }

        bool restore(int version)
        {
            std::unique_lock<std::shared_mutex> lock(sh_mtx);
            if (version < 0 || version >= versions.size())
            {
                return false;
            }

            // Save current state as a new version before restoring
            versions.push_back(currentRoot);

            // Set current root to a copy of the selected version's root
            currentRoot = versions[version];
            return true;
        }

        void serializeTrie(const shared_ptr<TrieNode>& node, string &trieString, string &idString, std::set<int> &st)
        {
            st.insert(node->m_id);  // ok
            idString += to_string(node->m_id);  // ok
            idString.push_back('[');
            if (node->isEndOfWord)
            {
                idString += to_string(node->index);
            }
            else
            {
                idString += "-1";
            }
            idString.push_back(']');
            for (auto child : node->children)
            {
                if(!(child.second)){
                    continue;
                }
                assert(child.second && "Child is NULL");
                trieString.push_back('[');
                trieString += to_string(node->m_id);
                trieString.push_back('>');
                trieString.push_back(child.first);
                trieString.push_back('>');
                trieString += to_string(child.second->m_id);
                trieString.push_back(']');
                if (st.find(child.second->m_id) == st.end())
                {
                    serializeTrie(child.second, trieString, idString, st);
                }
            }
        }

        

        std::vector<shared_ptr<TrieNode>> deserializeTrie(string &s, std::map<int ,shared_ptr<TrieNode>> &nodePointer)
        {
            std::vector<shared_ptr<TrieNode>> v{};
            std::size_t length{static_cast<std::size_t>(s.size())};
            for (std::size_t i{0}; i < length; ++i)
            {
                assert(s[i] != ']' && "Invalid serialized trie");
                std::size_t j{i+1};
                std::string parent{},child{};
                char c{};
                while(j < length && s[j]!='>'){
                    parent.push_back(s[j]);
                    ++j;
                }
                ++j;
                c=s[j];
                j+=2;
                while(j < length && s[j]!=']'){
                    child.push_back(s[j]);
                    ++j;
                }
                if(stoi(parent) == -1){
                    v.push_back(nodePointer[stoi(child)]);
                }
                else{
                    nodePointer[stoi(parent)]->children[c] = nodePointer[stoi(child)];
                }
                i = j;
            }
            return v;
        }

        void saveTrie(const std::string& filename)
        {
            std::ofstream fout(filename);
            for (auto &i : m_storage)
            {
                fout << i;
                fout << '>';
            }
            fout << '\n';
            std::string idString{}, trieString{};
            std::set<int> st;
            // std::cout << "Versions ki size: " << versions.size() << '\n';
            assert(currentRoot && "current root hi null hai");
            versions.push_back(currentRoot);
                    // int vid = 0;
            for (const auto& i : versions)
            {
                // std::cout << "currently version: " << vid++ << '\n';
                trieString.push_back('[');
                trieString += "-1";
                trieString.push_back('>');
                trieString.push_back('>');
                trieString.push_back('>');
                assert(i && "Null ho gya hu main");
                trieString += to_string(i->m_id);
                // std::cout << i->m_id << std::endl;
                trieString.push_back(']');
                serializeTrie(i, trieString, idString, st);
            }
            versions.pop_back();

            fout << idString << '\n';
            fout << trieString;
            fout.close();
        }

        // string serialize(){
        //     string s{};
        //     serializeTrie(currentRoot,s);
        //     s.pop_back();
        //     return s;
        // }
    };

}

#endif