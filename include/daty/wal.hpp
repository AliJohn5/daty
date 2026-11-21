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
};

class Wal {

  public:
    explicit Wal(std::filesystem::path path);

    void append(WalOperation operation, std::span<const std::byte> payload);

    std::vector<WalRecord> readAll() const;

  private:
    std::filesystem::path path_;
};

} // namespace daty