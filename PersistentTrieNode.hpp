#ifndef PERSISTENTTRIENODE_H
#define PERSISTENTTRIENODE_H

#include<string>
#include<unordered_map>
#include<optional>
#include<cassert>

class PersistentTrieNode{
public:
    PersistentTrieNode() = default;

    PersistentTrieNode* addChild(char c);

    void updateValue(std::string_view newValue = "");

    PersistentTrieNode* getChild(char c);

    bool hasChild() const ;

    bool hasValue() const;

    void removeChild(char c);

    std::optional<std::string> getValue() const ;

private:
    std::string m_value{};
    std::unordered_map<char, PersistentTrieNode*>m_child;
};

#endif