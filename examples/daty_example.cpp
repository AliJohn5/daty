
#include "daty/daty.hpp"

#include <cassert>
#include <iostream>
#include <string>

int main() {

    daty::Daty<std::string, std::string> users;

    users.put("username", "Ali");
    

    auto username = users.get("username");
    if (username) {
        std::cout << "Username: " << *username << std::endl;
    } else {
        std::cout << "Username not found!" << std::endl;    
    }



    auto country = users.get("country");
    if (country) {
        std::cout << "Country: " << *country << std::endl;
    } else {
        std::cout << "Country not found!" << std::endl;    
    }

    std::cout << "Size of hash table: " << users.size() << std::endl;

    users.remove("username");

    users.clear();

}
