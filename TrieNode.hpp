#ifndef TRIENODE_H
#define TRIENODE_H

#include <string>
#include <unordered_map>
#include <cassert>

namespace DB{

    class TrieNode{
    public:
        TrieNode() = default;

        TrieNode* addChild(char c);

        void updateValue(std::string_view newValue = "");

        TrieNode* getChild(char c);

        bool hasChild();

        bool hasValue();

        void removeChild(char c);
        
    private:
    
        std::string m_value{};
        std::unordered_map<char, TrieNode*>m_child;

    };

};

#endif