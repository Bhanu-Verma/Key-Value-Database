#ifndef TRIE_H
#define TRIE_H

#include "TrieNode.hpp"
#include <cassert>
#include <stack>

namespace DB{

    class Trie{
    public:
        Trie()
        : head{new TrieNode}
        {}
        
        void insert(std::string_view key, std::string_view value);

        bool remove(std::string_view key);

    private:
        TrieNode* head{nullptr};
        
    };

};

#endif