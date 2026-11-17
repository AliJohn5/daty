
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

    // Integer -> String
    daty::HashTable<int, std::string> employees;

    employees.insert(101, "Ali");
    employees.insert(102, "John");

    assert(employees.get(101) == "Ali");

    // String -> Integer
    daty::HashTable<std::string, int> ages;

    ages.insert("Ali", 25);
    ages.insert("John", 30);

    assert(ages.get("Ali") == 25);

    // Integer -> Double
    daty::HashTable<int, double> prices;

    prices.insert(1, 19.99);
    prices.insert(2, 49.99);

    assert(prices.get(1) == 19.99);

    std::cout << "All tests passed!\n";
}
