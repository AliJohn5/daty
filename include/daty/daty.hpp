#pragma once

#include "daty/snapshot.hpp"
#include "hash_table.hpp"
#include <cstddef>
#include <functional>
#include <optional>
#include <string>

namespace daty {

template <typename Key = std::string, typename Value = std::string, typename Hash = std::hash<Key>>
class Daty {

  private:
    HashTable<Key, Value, Hash> hash_table_;

  public:
    Daty() = default;
    ~Daty() = default;

    Daty(const Daty&) = delete;
    Daty& operator=(const Daty&) = delete;

    Daty(Daty&&) noexcept = default;
    Daty& operator=(Daty&&) noexcept = default;

    void save(std::string path) {
        Snapshot snapshout(path);
        snapshout.save(hash_table_);
    }

    void load(std::string path) {
        Snapshot snapshout(path);
        snapshout.load(hash_table_);
    }

    void put(const Key& key, const Value& value) {
        hash_table_.insert(key, value);
    }

    std::optional<Value> get(const Key& key) const {
        return hash_table_.get(key);
    }

    bool remove(const Key& key) {
        return hash_table_.remove(key);
    }

    std::size_t size() const {
        return hash_table_.size();
    }

    void clear() {
        hash_table_.clear();
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