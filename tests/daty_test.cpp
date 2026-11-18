
#include "daty/daty.hpp"

#include <cassert>
#include <iostream>
#include <string>



int main() {

    // String -> String
    daty::Daty<std::string, std::string> users;

    users.put("username", "Ali");
    users.put("country", "Netherlands");



    assert(users.get("username") == "Ali");


    // Integer -> String
    daty::Daty<int, std::string> employees;

    employees.put(101, "Ali");
    employees.put(102, "John");

    assert(employees.get(101) == "Ali");

    // String -> Integer
    daty::Daty<std::string, int> ages;

    ages.put("Ali", 25);
    ages.put("John", 30);

    assert(ages.get("Ali") == 25);

    // Integer -> Double
    daty::Daty<int, double> prices;

    prices.put(1, 19.99);
    prices.put(2, 49.99);

    assert(prices.get(1) == 19.99);


    daty::Daty<std::string, int> table;

    table.put("Ali", 25);
    table.put("John", 30);
    table.put("Alex", 22);

    for (auto it = table.begin(); it != table.end(); ++it) {
        std::cout << it->first << ": "
                  << it->second << '\n';
    }

    for (const auto& [key, value] : table) {
        std::cout << key << ": " << value << '\n';
    }


    std::cout << "All tests passed!\n";
}
