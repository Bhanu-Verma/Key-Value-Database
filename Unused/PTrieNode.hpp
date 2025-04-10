#ifndef PTRIENODE_H
#define PTRIENODE_H

#include <string>
#include <unordered_map>
#include <cassert>
#include <optional>

namespace DB
{

    class TrieNode
    {
    public:
        TrieNode() = default;

        TrieNode* addChild(char c)
        {
            m_child[c] = new TrieNode;
            m_child[c]->totParents = 1;
            return m_child[c];
        }

        TrieNode* addChild(char c, TrieNode* child){
            m_child[c] = child;
            m_child[c]->totParents = 1;
            return m_child[c];
        }

        void updateValue(std::string_view newValue = "")
        {
            m_value = newValue;
        }
        TrieNode *getChild(char c)
        {
            return (m_child.find(c) != m_child.end() ? m_child[c] : nullptr);
        }
        bool hasChild() const
        {
            return !(m_child.empty());
        }
        bool hasValue() const
        {
            return !(m_value.empty());
        }
        void removeChild(char c)
        {
            if (m_child.find(c) != m_child.end())
            {
                delete m_child[c];
                m_child.erase(c);
            }
        }
        std::optional<std::string> getValue() const
        {
            return (m_value == "" ? std::nullopt : std::optional<std::string>(m_value));
        }

        // writing copy constructor
        TrieNode(TrieNode& node){
            m_value = node.m_value;
            m_child = node.m_child;
            totParents = node.totParents;
            for(auto& i:m_child){
                if(i.second){
                    i.second->totParents++;
                }
            }
        }

        // make it in such a way that only Trie can access it as it should not be public
        void decreaseParents(){
            --totParents;
        }

        int getTotParents(){
            return totParents;
        }

        // void resetParents(){
        //     totParents = 1;
        // }

    private:
        std::string m_value{};
        std::unordered_map<char, TrieNode *> m_child;
        int totParents{};

        
    };

};

#endif