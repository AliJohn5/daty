
#include "daty/serializer.hpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <span>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

    using daty::Serializer;

// ============================================================
// Single-entry tests
// ============================================================

void test_normal_strings() {
    const std::string key = "Ali";
    const std::string value = "Yousef";

    std::vector<std::byte> bytes;
    std::size_t offset = 0;

    Serializer::serializeSingle(key, value, bytes, offset);

    assert(offset == bytes.size());

    offset = 0;

    auto [decoded_key, decoded_value] = Serializer::deserializeSingle(bytes, offset);

    assert(decoded_key == key);
    assert(decoded_value == value);
    assert(offset == bytes.size());
}

void test_empty_strings() {
    const std::string key;
    const std::string value;

    std::vector<std::byte> bytes;
    std::size_t offset = 0;

    Serializer::serializeSingle(key, value, bytes, offset);

    offset = 0;

    auto [decoded_key, decoded_value] = Serializer::deserializeSingle(bytes, offset);

    assert(decoded_key.empty());
    assert(decoded_value.empty());
    assert(offset == bytes.size());
}

void test_embedded_null_bytes() {
    const std::string key("key\0part", 8);
    const std::string value("value\0part", 10);

    std::vector<std::byte> bytes;
    std::size_t offset = 0;

    Serializer::serializeSingle(key, value, bytes, offset);

    offset = 0;

    auto [decoded_key, decoded_value] = Serializer::deserializeSingle(bytes, offset);

    assert(decoded_key == key);
    assert(decoded_value == value);
    assert(offset == bytes.size());
}

void test_binary_data() {
    const std::string key = {static_cast<char>(0xFF), static_cast<char>(0x80),
                             static_cast<char>(0x01)};

    const std::string value = {static_cast<char>(0xFE), static_cast<char>(0x00),
                               static_cast<char>(0x7F)};

    std::vector<std::byte> bytes;
    std::size_t offset = 0;

    Serializer::serializeSingle(key, value, bytes, offset);

    offset = 0;

    auto [decoded_key, decoded_value] = Serializer::deserializeSingle(bytes, offset);

    assert(decoded_key == key);
    assert(decoded_value == value);
    assert(offset == bytes.size());
}

void test_single_truncated_header() {
    std::vector<std::byte> bytes(10);
    std::size_t offset = 0;

    bool exception_thrown = false;

    try {
        Serializer::deserializeSingle(bytes, offset);
    } catch (const std::out_of_range&) {
        exception_thrown = true;
    }

    assert(exception_thrown);
}

// ============================================================
// Collection tests
// ============================================================

void test_empty_collection() {
    const std::vector<Serializer::Entry> entries;

    const auto bytes = Serializer::serialize(entries);

    const auto decoded = Serializer::deserialize(bytes);

    assert(decoded.empty());
}

void test_single_entry_collection() {
    const std::vector<Serializer::Entry> entries = {{"name", "Ali"}};

    const auto bytes = Serializer::serialize(entries);

    const auto decoded = Serializer::deserialize(bytes);

    assert(decoded.size() == 1);
    assert(decoded[0].first == "name");
    assert(decoded[0].second == "Ali");
}

void test_multiple_entries() {
    const std::vector<Serializer::Entry> entries = {
        {"name", "Ali"}, {"language", "C++"}, {"project", "daty"}, {"version", "1.0"}};

    const auto bytes = Serializer::serialize(entries);

    const auto decoded = Serializer::deserialize(bytes);

    assert(decoded == entries);
}

void test_collection_with_empty_strings() {
    const std::vector<Serializer::Entry> entries = {
        {"", ""}, {"key", ""}, {"", "value"}, {"normal", "entry"}};

    const auto bytes = Serializer::serialize(entries);

    const auto decoded = Serializer::deserialize(bytes);

    assert(decoded == entries);
}

void test_collection_with_binary_data() {
    const std::vector<Serializer::Entry> entries = {
        {std::string("key\0part", 8), std::string("value\0part", 10)},
        {std::string({static_cast<char>(0xFF), static_cast<char>(0x80)}),
         std::string({static_cast<char>(0xFE), static_cast<char>(0x01)})}};

    const auto bytes = Serializer::serialize(entries);

    const auto decoded = Serializer::deserialize(bytes);

    assert(decoded == entries);
}

// ============================================================
// Sequence tests
// ============================================================

void test_sequence_serialization() {
    const std::vector<Serializer::Entry> entries = {{"name", "Ali"}, {"project", "daty"}};

    const std::uint64_t expected_sequence = 42;

    const auto bytes = Serializer::serialize(entries, expected_sequence);

    std::uint64_t actual_sequence = 0;

    const auto decoded = Serializer::deserialize(bytes, &actual_sequence);

    assert(actual_sequence == expected_sequence);
    assert(decoded == entries);
}

void test_sequence_zero() {
    const std::vector<Serializer::Entry> entries = {{"key", "value"}};

    const auto bytes = Serializer::serialize(entries, 0);

    std::uint64_t sequence = 999;

    Serializer::deserialize(bytes, &sequence);

    assert(sequence == 0);
}

void test_sequence_max_uint64() {
    const std::vector<Serializer::Entry> entries = {{"key", "value"}};

    const std::uint64_t expected_sequence = std::numeric_limits<std::uint64_t>::max();

    const auto bytes = Serializer::serialize(entries, expected_sequence);

    std::uint64_t sequence = 0;

    Serializer::deserialize(bytes, &sequence);

    assert(sequence == expected_sequence);
}

void test_deserialize_without_sequence_pointer() {
    const std::vector<Serializer::Entry> entries = {{"key1", "value1"}, {"key2", "value2"}};

    const auto bytes = Serializer::serialize(entries, 123);

    const auto decoded = Serializer::deserialize(bytes);

    assert(decoded == entries);
}

// ============================================================
// Invalid collection tests
// ============================================================

void test_truncated_collection_header() {
    std::vector<std::byte> bytes(7);

    bool exception_thrown = false;

    try {
        Serializer::deserialize(bytes);
    } catch (const std::out_of_range&) {
        exception_thrown = true;
    }

    assert(exception_thrown);
}

void test_invalid_entry_count() {
    // An entry count of UINT64_MAX with no entry data.
    std::vector<std::byte> bytes(16, std::byte{0xFF});

    bool exception_thrown = false;

    try {
        Serializer::deserialize(bytes);
    } catch (const std::out_of_range&) {
        exception_thrown = true;
    }

    assert(exception_thrown);
}

void test_truncated_payload() {
    const std::vector<Serializer::Entry> entries = {{"hello", "world"}};

    auto bytes = Serializer::serialize(entries);

    bytes.pop_back();

    bool exception_thrown = false;

    try {
        Serializer::deserialize(bytes);
    } catch (const std::out_of_range&) {
        exception_thrown = true;
    }

    assert(exception_thrown);
}

void test_truncated_entry_header() {
    const std::vector<Serializer::Entry> entries = {{"hello", "world"}};

    auto bytes = Serializer::serialize(entries);

    // Remove part of the entry header.
    bytes.resize(16 + 10);

    bool exception_thrown = false;

    try {
        Serializer::deserialize(bytes);
    } catch (const std::out_of_range&) {
        exception_thrown = true;
    }

    assert(exception_thrown);
}

void test_trailing_bytes() {
    const std::vector<Serializer::Entry> entries = {{"hello", "world"}};

    auto bytes = Serializer::serialize(entries);

    bytes.push_back(std::byte{0x42});

    bool exception_thrown = false;

    try {
        Serializer::deserialize(bytes);
    } catch (const std::runtime_error&) {
        exception_thrown = true;
    }

    assert(exception_thrown);
}

// ============================================================
// Main
// ============================================================

int main() {
    // Single-entry tests
    test_normal_strings();
    test_empty_strings();
    test_embedded_null_bytes();
    test_binary_data();
    test_single_truncated_header();

    // Collection tests
    test_empty_collection();
    test_single_entry_collection();
    test_multiple_entries();
    test_collection_with_empty_strings();
    test_collection_with_binary_data();

    // Sequence tests
    test_sequence_serialization();
    test_sequence_zero();
    test_sequence_max_uint64();
    test_deserialize_without_sequence_pointer();

    // Invalid-data tests
    test_truncated_collection_header();
    test_invalid_entry_count();
    test_truncated_payload();
    test_truncated_entry_header();
    test_trailing_bytes();

    std::cout << "All Serializer tests passed!\n";

    return 0;
}
