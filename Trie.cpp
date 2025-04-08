#include "Trie.hpp"




void DB::Trie::insert(std::string_view key, std::string_view value){
    assert(key.size() && value.size() && "key and value should be nonempty");
    TrieNode * cur{head};
    for(auto i : key){
        if(cur->getChild(i)){
            cur = cur->getChild(i);
        }
        else{
            cur = cur->addChild(i);
        }
    }
    cur->updateValue(value);

}


bool DB::Trie::remove(std::string_view key){
    assert(key.size() && "key should be nonempty");
    std::stack<std::pair<char, TrieNode*>> st;
    TrieNode * cur{head};
    for(auto i : key){
        st.push({i, cur});
        if(cur->getChild(i)){
            cur = cur->getChild(i);
        }
        else{
            return false;
        }
    }
    cur->updateValue();
    if(!(cur->hasChild())){
        delete cur;
        while(true){
            auto p{st.top()};
            st.pop();
            p.second->removeChild(p.first);
            if(st.empty() || p.second->hasChild() || p.second->hasValue()){
                break;
            }
            delete p.second;
        }
    }
    return true;
}

std::optional<std::string> DB::Trie::get(std::string key) const
{
    TrieNode* cur{head};
    for(auto i : key){
        if(cur->getChild(i)){
            cur = cur->getChild(i);
        }
        else{
            return std::nullopt;
        }
    }
    return cur->getValue();
}

bool DB::Trie::exists(std::string_view key) const {
    TrieNode* cur{head};
    for(auto i : key){
        if(cur->getChild(i)){
            cur = cur->getChild(i);
        }
        else{
            return false;
        }
    }
    return cur->hasValue();
}

