
#include "daty/serializer.hpp"

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>

namespace daty {

void Serializer::write_uint64(std::vector<std::byte>& buffer, std::uint64_t value,
                              std::size_t& offset) {
    if (offset > buffer.max_size() - UINT64_SIZE) {
        throw std::length_error("Buffer too large");
    }

    const std::size_t required_size = offset + UINT64_SIZE;

    if (buffer.size() < required_size) {
        buffer.resize(required_size);
    }

    // Write the integer in little-endian format.
    for (std::size_t i = 0; i < UINT64_SIZE; ++i) {
        buffer[offset + i] = static_cast<std::byte>(value & 0xFF);
        value >>= 8;
    }

    offset += UINT64_SIZE;
}

std::uint64_t Serializer::read_uint64(std::span<const std::byte> data, std::size_t& offset) {
    if (offset > data.size() || data.size() - offset < UINT64_SIZE) {
        throw std::out_of_range("Not enough bytes to read uint64");
    }

    std::uint64_t result = 0;

    // Read the integer from little-endian format.
    for (std::size_t i = 0; i < UINT64_SIZE; ++i) {
        result |= static_cast<std::uint64_t>(std::to_integer<std::uint8_t>(data[offset + i]))
                  << (i * 8);
    }

    offset += UINT64_SIZE;

    return result;
}

void Serializer::write_string(std::vector<std::byte>& buffer, const std::string& value,
                              std::size_t& offset) {
    if (value.size() > buffer.max_size() || offset > buffer.max_size() - value.size()) {
        throw std::length_error("Buffer too large");
    }

    const std::size_t required_size = offset + value.size();

    if (buffer.size() < required_size) {
        buffer.resize(required_size);
    }

    for (std::size_t i = 0; i < value.size(); ++i) {
        buffer[offset + i] = static_cast<std::byte>(static_cast<unsigned char>(value[i]));
    }

    offset += value.size();
}

std::string Serializer::read_string(std::span<const std::byte> data, std::size_t& offset,
                                    std::size_t length) {
    if (offset > data.size() || data.size() - offset < length) {
        throw std::out_of_range("Not enough bytes to read string");
    }

    std::string result(length, '\0');

    for (std::size_t i = 0; i < length; ++i) {
        result[i] = static_cast<char>(std::to_integer<unsigned char>(data[offset + i]));
    }

    offset += length;

    return result;
}

void Serializer::serializeSingle(const std::string& key, const std::string& value,
                                 std::vector<std::byte>& bytes, std::size_t& offset) {
    // Ensure string lengths fit in uint64_t.
    if constexpr (sizeof(std::size_t) > sizeof(std::uint64_t)) {
        if (key.size() > std::numeric_limits<std::uint64_t>::max() ||
            value.size() > std::numeric_limits<std::uint64_t>::max()) {
            throw std::length_error("String too large to serialize");
        }
    }

    write_uint64(bytes, static_cast<std::uint64_t>(key.size()), offset);

    write_uint64(bytes, static_cast<std::uint64_t>(value.size()), offset);

    write_string(bytes, key, offset);
    write_string(bytes, value, offset);
}

std::pair<std::string, std::string> Serializer::deserializeSingle(std::span<const std::byte> data,
                                                                  std::size_t& offset) {

    if (offset > data.size()) {
        throw std::out_of_range("Invalid serialization offset");
    }

    const std::size_t allRemaining = data.size() - offset;

    if (allRemaining < (2 * UINT64_SIZE)) {
        throw std::out_of_range("Invalid serialized entry count");
    }

    // Read the two lengths from the header.
    const std::uint64_t key_length = read_uint64(data, offset);
    const std::uint64_t value_length = read_uint64(data, offset);

    // Ensure lengths fit in size_t before converting.
    if constexpr (sizeof(std::size_t) < sizeof(std::uint64_t)) {
        if (key_length > std::numeric_limits<std::size_t>::max() ||
            value_length > std::numeric_limits<std::size_t>::max()) {
            throw std::length_error("Serialized string length exceeds size_t");
        }
    }

    const std::size_t key_size = static_cast<std::size_t>(key_length);

    const std::size_t value_size = static_cast<std::size_t>(value_length);

    // Validate the payload lengths before allocating strings.
    const std::size_t remaining = data.size() - offset;

    if (key_size > remaining || value_size > remaining - key_size) {
        throw std::out_of_range("Invalid serialized record");
    }

    std::string key = read_string(data, offset, key_size);
    std::string value = read_string(data, offset, value_size);

    return {std::move(key), std::move(value)};
}

std::vector<std::byte> Serializer::serialize(const std::vector<Entry>& data,
                                             std::uint64_t db_sequence) {

    std::vector<std::byte> result;
    std::size_t offset = 0;

    write_uint64(result, db_sequence, offset);

    if constexpr (sizeof(std::size_t) > sizeof(std::uint64_t)) {
        if (data.size() > std::numeric_limits<std::uint64_t>::max()) {
            throw std::length_error("Data too large to serialize");
        }
    }
    write_uint64(result, static_cast<std::uint64_t>(data.size()), offset);

    for (const auto& [key, value] : data) {
        serializeSingle(key, value, result, offset);
    }

    return result;
}

std::vector<Serializer::Entry> Serializer::deserialize(std::span<const std::byte> data,
                                                       std::uint64_t* db_sequence) {

    if (data.size() < UINT64_SIZE) {
        throw std::out_of_range("Invalid serialized entry count");
    }

    std::size_t offset = 0;
    const std::uint64_t sequence = read_uint64(data, offset);

    if (db_sequence != nullptr) {
        *db_sequence = sequence;
    }

    // Read number of pairs
    const std::uint64_t data_length = read_uint64(data, offset);

    const std::size_t remaining = data.size() - offset;

    if (data_length > remaining / (2 * UINT64_SIZE)) {
        throw std::out_of_range("Invalid serialized entry count");
    }

    std::vector<Serializer::Entry> ans;
    ans.reserve(static_cast<std::size_t>(data_length) + 1);

    for (std::size_t i = 0; i < data_length; ++i) {
        auto deserializedData = deserializeSingle(data, offset);
        ans.emplace_back(std::move(deserializedData));
    }

    // Reject unexpected trailing bytes.

    if (offset != data.size()) {
        throw std::runtime_error("Unexpected trailing bytes");
    }

    return ans;
}

} // namespace daty
