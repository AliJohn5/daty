
#include "daty/serializer.hpp"

#include <cassert>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using daty::Serializer;

void test_normal_strings() {
    const std::string key = "Ali";
    const std::string value = "Yousef";

    const auto serialized = Serializer::serialize(key, value);
    const auto [decoded_key, decoded_value] =
        Serializer::deserialize(serialized);

    assert(decoded_key == key);
    assert(decoded_value == value);
}

void test_empty_strings() {
    const std::string key;
    const std::string value;

    const auto serialized = Serializer::serialize(key, value);
    const auto [decoded_key, decoded_value] =
        Serializer::deserialize(serialized);

    assert(decoded_key == key);
    assert(decoded_value == value);
}

void test_embedded_null_bytes() {
    const std::string key("key\0part", 8);
    const std::string value("value\0part", 10);

    const auto serialized = Serializer::serialize(key, value);
    const auto [decoded_key, decoded_value] =
        Serializer::deserialize(serialized);

    assert(decoded_key == key);
    assert(decoded_value == value);
}

void test_binary_data() {
    const std::string key = {
        static_cast<char>(0xFF),
        static_cast<char>(0x80),
        static_cast<char>(0x01)
    };

    const std::string value = {
        static_cast<char>(0xFE),
        static_cast<char>(0x00),
        static_cast<char>(0x7F)
    };

    const auto serialized = Serializer::serialize(key, value);
    const auto [decoded_key, decoded_value] =
        Serializer::deserialize(serialized);

    assert(decoded_key == key);
    assert(decoded_value == value);
}

void test_truncated_header() {
    std::vector<std::byte> data(10);

    bool exception_thrown = false;

    try {
        Serializer::deserialize(data);
    }
    catch (const std::out_of_range&) {
        exception_thrown = true;
    }

    assert(exception_thrown);
}

void test_truncated_payload() {
    auto data = Serializer::serialize("hello", "world");

    data.pop_back();

    bool exception_thrown = false;

    try {
        Serializer::deserialize(data);
    }
    catch (const std::out_of_range&) {
        exception_thrown = true;
    }

    assert(exception_thrown);
}

void test_trailing_bytes() {
    auto data = Serializer::serialize("hello", "world");

    data.push_back(std::byte{0x42});

    bool exception_thrown = false;

    try {
        Serializer::deserialize(data);
    }
    catch (const std::runtime_error&) {
        exception_thrown = true;
    }

    assert(exception_thrown);
}

int main() {
    test_normal_strings();
    test_empty_strings();
    test_embedded_null_bytes();
    test_binary_data();
    test_truncated_header();
    test_truncated_payload();
    test_trailing_bytes();

    std::cout << "All Serializer tests passed!\n";

    return 0;
}
