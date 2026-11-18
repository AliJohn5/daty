
#include "daty/hash_table.hpp"

#include <cassert>
#include <iostream>
#include <string>

int main() {

    // String -> String
    daty::HashTable<std::string, std::string> users;

    users.insert("username", "Ali");
    users.insert("country", "Netherlands");

    assert(users.get("username") == "Ali");
    assert(users.get("no-value") == std::nullopt);


    // Integer -> String
    daty::HashTable<int, std::string> employees;

    employees.insert(101, "Ali");
    employees.insert(102, "John");

    assert(employees.get(101) == "Ali");
    assert(employees.get(103) == std::nullopt);


    // String -> Integer
    daty::HashTable<std::string, int> ages;

    ages.insert("Ali", 25);
    ages.insert("John", 30);

    assert(ages.get("Ali") == 25);
    assert(ages.get("no-value") == std::nullopt);

    // Integer -> Double
    daty::HashTable<int, double> prices;

    prices.insert(1, 19.99);
    prices.insert(2, 49.99);

    assert(prices.get(1) == 19.99);
    assert(prices.get(3) == std::nullopt);


    daty::HashTable<std::string, int> db;

    db.insert("age", 25);

    // Using get()
    auto age = db.get("age");
    *age = 25;
    assert(db.get("age") == 25);

    // Using at()
    db.at("age") = 35;
    assert(db.get("age") == 35);

    try {
        db.at("unknown");
    } catch (const std::out_of_range& e) {
        std::cout << e.what() << '\n';
    }


    daty::HashTable<std::string, int> table;

    table.insert("Ali", 25);
    table.insert("John", 30);
    table.insert("Alex", 22);

    for (auto it = table.begin(); it != table.end(); ++it) {
        std::cout << it->first << ": "
                  << it->second << '\n';
    }

    for (const auto& [key, value] : table) {
        std::cout << key << ": " << value << '\n';
    }

    std::cout << "All tests passed!\n";
}
