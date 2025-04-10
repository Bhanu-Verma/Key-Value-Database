#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <optional>
#include <random>
#include <cassert>
#include "Trie.hpp"
using namespace std;

// Generate random lowercase string
std::string randomString(size_t length) {
    static const char charset[] = "abcdefghijklmnopqrstuvwxyz";
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<size_t> dist(0, sizeof(charset) - 2);

    std::string result;
    for (size_t i = 0; i < length; ++i)
        result += charset[dist(rng)];
    return result;
}

void testTrieWithVersioning(size_t operations = 10000) {
    DB::PersistentTrie trie;
    std::unordered_map<std::string, std::string> reference;
    std::vector<std::unordered_map<std::string, std::string>> mapVersions;
    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> opDist(0, 10); // 0-9 range (for controlling commit/restore probability)

    for (size_t i = 0; i < operations; ++i) {
        std::cout << i+1 << ") ";
        int op = opDist(rng);
        std::string key = randomString(1 + rng() % 2); // key: 5–9 chars

        if (op <= 4) {
            // INSERT (50% chance)
            std::string value = randomString(1 + rng() % 30);
            trie.insert(key, value);
            reference[key] = value;
            std::cout << "Key: " << key << "   value: " << value << '\n';
            auto trieVal = trie.get(key);
            assert(trieVal.has_value());
            assert(trieVal.value() == value);
        } else if (op <= 7) {
            // REMOVE (30% chance)
            bool removedMap = reference.erase(key) > 0;
            if(trie.exists(key))
                trie.remove(key);
            assert(!trie.exists(key));
            std::cout << "Key: " << key <<'\n';

            auto trieVal = trie.get(key);
            assert(!trieVal.has_value());
        } else if (op == 8) {
            // COMMIT (10% chance)
            trie.commit();
            mapVersions.push_back(reference);
            std::cout << "[Commit] Version " << mapVersions.size() - 1 << " saved\n";
        } else if (op == 9) {
            if(mapVersions.empty()) continue;
            // RESTORE (10% chance if versions exist)
            std::uniform_int_distribution<size_t> restoreDist(0, mapVersions.size() - 1);
            size_t version = restoreDist(rng);

            trie.restore(version);
            mapVersions.push_back(reference);
            reference = mapVersions[version];
            std::cout << "[Restore] Restored to version " << version << "\n";
        }
        else{
            std::cout << "I'm saving and loading from the file.\n";
            trie.saveTrie("SerialTrie.bin");
            trie.loadTrieFromFile("SerialTrie.bin");
        }

        // Final check after each operation
        for (const auto& [key, value] : reference) {
            auto trieVal = trie.get(key);
            if(!trieVal.has_value() || !(trieVal.value()==reference[key])){
                std::cout << "dikkat yha hai: " << key << '\n';
                std::cout << "Expected: " << reference[key] << '\n';
                std::cout << "Found:";
                if(!trieVal.has_value()){
                    std::cout << "not found\n";
                }
                else{
                    std::cout << (trieVal.value()) << '\n';
                }
            }
            assert(trieVal.has_value());
            assert(trieVal.value() == reference[key]);
        }
        // Also make sure no extra keys are in trie
        // (optional, depends on your Trie definition — we can skip this if your Trie does not support key iteration)
    }

    std::cout << "All operations including commit/restore validated successfully!" << std::endl;
}

int main() {
    testTrieWithVersioning();
    return 0;
}