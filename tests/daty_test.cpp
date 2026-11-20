
#include "daty/daty.hpp"

#include <cassert>
#include <iostream>
#include <map>
#include <ostream>
#include <string>

int main() {

    // String -> String
    daty::Daty<std::string, std::string> originDB;
    originDB.put("name", "Ali");
    originDB.put("age", "26");
    originDB.save("test.snapshot");

    daty::Daty<std::string, std::string> restoredDB;
    restoredDB.load("test.snapshot");
    restoredDB.put("last", "john");
    restoredDB.save("test.snapshot");

    std::map<std::string, std::string> mp;
    mp["name"] = "Ali";
    mp["age"] = "26";
    mp["last"] = "john";

    for (const auto& [k, v] : restoredDB) {
        std::cout << k << " " << v << std::endl;
        assert(mp[k] == v);
        mp.erase(k);
    }

    assert(mp.size() == 0);
    std::cout << "All tests passed!\n";
}
