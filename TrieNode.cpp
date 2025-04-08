#include "TrieNode.hpp"

DB::TrieNode* DB::TrieNode::addChild(char c){
    return m_child[c] = new TrieNode;
}

void DB::TrieNode::updateValue(std::string_view newValue){
    m_value = newValue;
}

DB::TrieNode *DB::TrieNode::getChild(char c){
    return (m_child.find(c) != m_child.end() ? m_child[c] : nullptr);
}

bool DB::TrieNode::hasChild(){
    return !(m_child.empty());
}

bool DB::TrieNode::hasValue(){
    return !(m_value.empty());
}

void DB::TrieNode::removeChild(char c){
    if(m_child.find(c) != m_child.end()){
        m_child.erase(c);
    }
}
