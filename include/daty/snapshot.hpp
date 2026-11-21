
#pragma once

#include "daty/hash_table.hpp"
#include <cstdint>
#include <filesystem>
#include <string>

namespace daty {

class Snapshot {
  public:
    explicit Snapshot(const std::string& filePath);

    void save(const HashTable<std::string, std::string>& table);
    void load(HashTable<std::string, std::string>& table);

    std::uint64_t db_sequence = 0;

  private:
    std::filesystem::path filePath_;
};
} // namespace daty
