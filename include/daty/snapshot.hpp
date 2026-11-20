
#pragma once

#include <string>
#include "daty/hash_table.hpp"

namespace daty {

class Snapshot {
  public:
    explicit Snapshot(const std::string& filePath);

    void save(const HashTable<std::string,std::string>& table);
    void load(HashTable<std::string, std::string>& table);

  private:
    std::string filePath_;
};
} // namespace daty
