
#include "daty/daty.hpp"

#include <cassert>
#include <iostream>
#include <string>
#include <time.h>
#include <map>



struct ConstantHash {
    std::size_t operator()(const std::string&) const {
        return 0;
    }
};


void test_collisions() {
    daty::Daty<std::string, int, ConstantHash> db;

    db.put("Ali", 25);
    db.put("John", 30);
    db.put("Alex", 22);
    db.put("Mike", 35);
    db.put("Sarah", 28);

    assert(db.size() == 5);

    std::map<std::string, int> reference = {
        {"Ali", 25},
        {"John", 30},
        {"Alex", 22},
        {"Mike", 35},
        {"Sarah", 28}
    };

    for (const auto& [key, value] : db) {
        auto it = reference.find(key);

        assert(it != reference.end());
        assert(it->second == value);

        reference.erase(it);
    }

    assert(reference.empty());


    db.put("Ali", 100);       // Update existing key

    assert(db.at("Ali") == 100);
    
    db.remove("Alex");        // Remove middle node
    db.remove("Ali");         // Remove head node
    db.remove("Sarah");       // Remove another node
    
    assert(db.size() == 2);

    std::cout << "Collision test passed!\n";
}

int main() {

    daty::Daty<std::string, int> db;

    // test empty db
    assert(db.begin() == db.end());

    // test one element db
    db.put("Ali", 123);
    assert(db.begin() != db.end());
    auto it = db.begin();
    it++;
    assert(it == db.end());


    // test after remove element db
    db.remove("Ali");
    assert(db.begin() == db.end());


    // test many elements
    srand(time(0));
    std::size_t testCases = 10000;
    std::map<std::string, int> reference;

    for (std::size_t i = 0; i < testCases; ++i) {
        int randomNumber = rand() % 10000;
        std::string randomString = std::to_string(randomNumber);

        reference[randomString] = randomNumber;
        db.put(randomString, randomNumber);
        assert(*db.get(randomString) == reference[randomString]);
    }

    assert(db.size() == reference.size());

    for (const auto& [key, value] : db) {
        auto it = reference.find(key);

        assert(it != reference.end());
        assert(it->second == value);

        reference.erase(it);
    }

    assert(reference.empty());



    test_collisions();


    std::cout << "All tests passed!\n";
}
