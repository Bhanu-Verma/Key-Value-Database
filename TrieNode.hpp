#ifndef TRIENODE_H
#define TRIENODE_H

#include <string>
#include <unordered_map>
#include <cassert>
#include <optional>

namespace DB{

    class TrieNode{
    public:
        TrieNode() = default;

        TrieNode* addChild(char c);

        void updateValue(std::string_view newValue = "");

        TrieNode* getChild(char c);

        bool hasChild() const ;

        bool hasValue() const;

        void removeChild(char c);

        std::optional<std::string> getValue() const ;
        
    private:
    
        std::string m_value{};
        std::unordered_map<char, TrieNode*>m_child;

    };

};

#endif