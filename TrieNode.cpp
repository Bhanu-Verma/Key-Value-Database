#include "TrieNode.hpp"

DB::TrieNode* DB::TrieNode::addChild(char c){
    return m_child[c] = new TrieNode;
}

void DB::TrieNode::updateValue(std::string_view newValue){
    m_value = newValue;
}

DB::TrieNode*  DB::TrieNode::getChild(char c){
    return (m_child.find(c) != m_child.end() ? m_child[c] : nullptr);
}

bool DB::TrieNode::hasChild() const {
    return !(m_child.empty());
}

bool DB::TrieNode::hasValue() const {
    return !(m_value.empty());
}

void DB::TrieNode::removeChild(char c){
    if(m_child.find(c) != m_child.end()){
        delete m_child[c];
        m_child.erase(c);
    }
}

std::optional<std::string> DB::TrieNode::getValue() const {
    return (m_value == "" ? std::nullopt : std::optional<std::string>(m_value));
}
