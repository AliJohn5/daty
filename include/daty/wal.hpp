#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <span>
#include <vector>

namespace daty {

enum class WalOperation : std::uint8_t { PUT = 1, DELETE = 2, CLEAR = 3 };

struct WalRecord {
    WalOperation operation;
    std::vector<std::byte> payload;
    std::uint64_t sequence;
};

class Wal {

  public:
    explicit Wal(std::filesystem::path path);

    void append(WalOperation operation, std::span<const std::byte> payload);

    void setDataBaseSequence(std::uint64_t db_sequence);

    std::vector<WalRecord> readAll() const;

  private:
    std::filesystem::path path_;
    std::uint64_t sequence_ = 1;
    std::uint64_t db_sequence_ = 0;
};

} // namespace daty