#include <thread>
#include <vector>
#include <random>
#include "..\Trie.hpp"

void writerThread(DB::PersistentTrie& db, int id) {
    for (int i = 0; i < 10; i++) {
        db.insert("key" + std::to_string(id) + "_" + std::to_string(i), "val" + std::to_string(i));
    }
}

void readerThread(DB::PersistentTrie& db, int id) {
    for (int i = 0; i < 10; i++) {
        db.get("key" + std::to_string(rand() % 5) + "_" + std::to_string(rand() % 1000));
    }
}

int main() {
    DB::PersistentTrie db{"db1", "bhanu"};
    std::vector<std::thread> threads;

    for (int i = 0; i < 5; i++){
        threads.emplace_back(writerThread, std::ref(db), i);
        threads.emplace_back(readerThread, std::ref(db), i);
    }

    for (auto& t : threads) t.join();

    return 0;
}
