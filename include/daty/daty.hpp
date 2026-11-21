
#pragma once

#include "daty/serializer.hpp"
#include "daty/snapshot.hpp"
#include "daty/wal.hpp"
#include "hash_table.hpp"

#include <cstddef>
#include <filesystem>
#include <functional>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace daty {

template <typename Key = std::string, typename Value = std::string, typename Hash = std::hash<Key>>
class Daty {

  private:
    HashTable<Key, Value, Hash> hash_table_;
    std::optional<Wal> wal_;
    std::string path_;

    void replayWal() {
        if (!wal_) {
            return;
        }

        const auto records = wal_->readAll();

        for (const auto& record : records) {

            if (record.operation == WalOperation::CLEAR) {
                hash_table_.clear();
                continue;
            }

            std::size_t offset = 0;

            const auto [key, value] = Serializer::deserializeSingle(record.payload, offset);

            if (offset != record.payload.size()) {
                throw std::runtime_error("Invalid WAL record payload");
            }

            if (record.operation == WalOperation::PUT) {
                hash_table_.insert(key, value);
            } else if (record.operation == WalOperation::DELETE) {
                hash_table_.remove(key);
            }
        }
    }

    void appendPut(const std::string& key, const std::string& value) {
        if (!wal_) {
            return;
        }

        std::vector<std::byte> payload;
        std::size_t offset = 0;

        Serializer::serializeSingle(key, value, payload, offset);

        wal_->append(WalOperation::PUT, payload);
        hash_table_.increaseOperationCounter();
    }

    void appendDelete(const std::string& key) {
        if (!wal_) {
            return;
        }

        std::vector<std::byte> payload;
        std::size_t offset = 0;

        const std::string emptyValue;

        Serializer::serializeSingle(key, emptyValue, payload, offset);

        wal_->append(WalOperation::DELETE, payload);
        hash_table_.increaseOperationCounter();
    }

  public:
    Daty() = default;
    ~Daty() {
        if constexpr (std::is_same_v<Key, std::string> && std::is_same_v<Value, std::string>) {
            try {
                if (!path_.empty())
                    save(path_);
            } catch (const std::exception& e) {
                std::cerr << "Failed to save Daty: " << e.what() << '\n';
            }
        }
    }

    Daty(const Daty&) = delete;
    Daty& operator=(const Daty&) = delete;

    Daty(Daty&&) noexcept = default;
    Daty& operator=(Daty&&) noexcept = default;

    // --------------------------------------------------
    // Persistence
    // --------------------------------------------------

    void open(const std::filesystem::path& path) {

        static_assert(std::is_same_v<Key, std::string> && std::is_same_v<Value, std::string>,
                      "WAL persistence currently supports only string keys and values");

        hash_table_.clear();

        const auto walPath = std::filesystem::path(path.string() + ".wal");

        wal_.emplace(walPath);

        if (std::filesystem::exists(path)) {
            Snapshot snapshot(path.string());
            snapshot.load(hash_table_);
            wal_->setDataBaseSequence(snapshot.db_sequence);
        }

        replayWal();
    }

    void save(std::string path) {
        Snapshot snapshot(path);
        snapshot.save(hash_table_);
    }

    void load(std::string path) {
        path_ = path;
        open(path_);
    }

    // --------------------------------------------------
    // CRUD
    // --------------------------------------------------

    void put(const Key& key, const Value& value) {
        if constexpr (std::is_same_v<Key, std::string> && std::is_same_v<Value, std::string>) {
            appendPut(key, value);
        }

        hash_table_.insert(key, value);
    }

    std::optional<Value> get(const Key& key) const {
        return hash_table_.get(key);
    }

    bool remove(const Key& key) {
        if (!hash_table_.contains(key)) {
            return false;
        }

        if constexpr (std::is_same_v<Key, std::string> && std::is_same_v<Value, std::string>) {
            appendDelete(key);
        }

        return hash_table_.remove(key);
    }

    void clear() {
        if (wal_) {
            const std::vector<std::byte> emptyPayload;
            wal_->append(WalOperation::CLEAR, emptyPayload);
            hash_table_.increaseOperationCounter();
        }

        hash_table_.clear();
    }

    // --------------------------------------------------
    // Queries
    // --------------------------------------------------

    std::size_t size() const {
        return hash_table_.size();
    }

    bool empty() const {
        return hash_table_.empty();
    }

    bool contains(const Key& key) const {
        return hash_table_.contains(key);
    }

    std::size_t bucket_count() const {
        return hash_table_.bucket_count();
    }

    Value& at(const Key& key) {
        return hash_table_.at(key);
    }

    const Value& at(const Key& key) const {
        return hash_table_.at(key);
    }

    typename HashTable<Key, Value, Hash>::Iterator begin() {
        return hash_table_.begin();
    }

    typename HashTable<Key, Value, Hash>::Iterator end() {
        return hash_table_.end();
    }
};

} // namespace daty
