
#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace daty {

class Serializer {
  public:
    using Entry = std::pair<std::string, std::string>;

    static std::vector<std::byte> serialize(const std::vector<Entry>& data,
                                            std::uint64_t db_sequence = 0);

    static std::vector<Entry> deserialize(std::span<const std::byte> data,
                                          std::uint64_t* db_sequence = nullptr);

    static void serializeSingle(const std::string& key, const std::string& value,
                                std::vector<std::byte>& bytes, std::size_t& offset);

    static std::pair<std::string, std::string> deserializeSingle(std::span<const std::byte> data,
                                                                 std::size_t& offset);

  private:
    static constexpr std::size_t UINT64_SIZE = sizeof(std::uint64_t);

    static void write_uint64(std::vector<std::byte>& buffer, std::uint64_t value,
                             std::size_t& offset);

    static std::uint64_t read_uint64(std::span<const std::byte> data, std::size_t& offset);

    static void write_string(std::vector<std::byte>& buffer, const std::string& value,
                             std::size_t& offset);

    static std::string read_string(std::span<const std::byte> data, std::size_t& offset,
                                   std::size_t length);
};

} // namespace daty
