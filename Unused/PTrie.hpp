#ifndef PTRIE_H
#define PTRIE_H

#include "PTrieNode.hpp"
#include <cassert>
#include <stack>
#include <optional>
#include <vector>

namespace DB
{

    class Trie
    {
    public:
        Trie()
            : m_versions{new TrieNode}
        {
        }

        void insert(std::string_view key, std::string_view value)
        {
            assert(key.size() && value.size() && "key and value should be nonempty");
            TrieNode *cur{m_versions.back()};
            for (auto i : key)
            {
                TrieNode* child{cur->getChild(i)};
                if (child)
                {
                    if(child->getTotParents() != 1){
                        child->decreaseParents();
                        child = new TrieNode(*child);
                        cur->addChild(i, child);
                    }
                    cur = child;
                }
                else
                {
                    cur = cur->addChild(i);
                }
            }
            cur->updateValue(value);
        }

        bool remove(std::string_view key)
        {
            assert(key.size() && "key should be nonempty");
            std::stack<std::pair<char, TrieNode *>> st;
            TrieNode *cur{m_versions.back()};
            for (auto i : key)
            {
                st.push({i, cur});
                TrieNode* child{cur->getChild(i)};
                if (child)
                {
                    if(child->getTotParents() != 1){
                        child->decreaseParents();
                        child = new TrieNode(*child);
                        cur->addChild(i, child);
                    }
                    cur = child;
                }
                else
                {
                    return false;
                }
            }
            cur->updateValue();
            if (!(cur->hasChild()))
            {
                while (!st.empty())
                {
                    auto p{st.top()};
                    st.pop();
                    p.second->removeChild(p.first);
                    if (p.second->hasChild() || p.second->hasValue())
                    {
                        break;
                    }
                }
            }
            return true;
        }

        std::optional<std::string> get(std::string key)
        {
            TrieNode *cur{head};
            for (auto i : key)
            {
                if (cur->getChild(i))
                {
                    cur = cur->getChild(i);
                }
                else
                {
                    return std::nullopt;
                }
            }
            return cur->getValue();
        }
        bool exists(std::string_view key) const
        {
            TrieNode *cur{head};
            for (auto i : key)
            {
                if (cur->getChild(i))
                {
                    cur = cur->getChild(i);
                }
                else
                {
                    return false;
                }
            }
            return cur->hasValue();
        }

        void commit(){
            m_versions.push_back(new TrieNode{*m_versions.back()});
        }

        void restore(int version){
            assert(version < m_versions.size() && "no such version exists yet");
            m_versions.push_back(new TrieNode{*m_versions[version]});
        }
        
    private:
        std::vector<TrieNode*> m_versions{};
        TrieNode *head{nullptr};
    };

};

#endif