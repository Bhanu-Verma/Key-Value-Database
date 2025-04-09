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
using namespace std;

namespace DB{
    struct TrieNode {
        std::string value{};
        bool isEndOfWord{};
        std::unordered_map<char, shared_ptr<TrieNode>> children{};

        TrieNode(bool isEnd = false)
        : isEndOfWord{ isEnd }
        {}
    };

    class PersistentTrie {
    private: 
        vector<shared_ptr<TrieNode>> versions;
        shared_ptr<TrieNode> currentRoot;
        mutable std::shared_mutex sh_mtx;


        shared_ptr<TrieNode> insert(shared_ptr<TrieNode> node, const string &word, int index, std::string_view val) {



            if (!node) node = make_shared<TrieNode>();

            shared_ptr<TrieNode> newNode = make_shared<TrieNode>(*node); // copy current node

            if (index == word.size()) {
                newNode->isEndOfWord = true;
                newNode->value = val;
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
        
            shared_ptr<TrieNode> newNode = make_shared<TrieNode>(*node); // copy for persistence
        
            if (index == key.size()) {
                if (!newNode->isEndOfWord) return newNode; // key doesn't exist
                newNode->isEndOfWord = false;
                newNode->value.clear();
                return newNode;
            }
        
            char ch = key[index];
            newNode->children[ch] = remove(newNode->children[ch], key, index + 1);
            return newNode;
        }
        

    public:
        PersistentTrie() {
            currentRoot = make_shared<TrieNode>();
        }

        void insert(const string &word, std::string_view val) {
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
            if (node && node->isEndOfWord) {
                return node->value;
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
    };
}


#endif