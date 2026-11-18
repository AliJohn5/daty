#ifndef DATY_HPP
#define DATY_HPP

#include <cstddef>
#include <functional>
#include <optional>
#include "hash_table.hpp"

namespace daty {

    template <
        typename Key,
        typename Value,
        typename Hash = std::hash<Key>
    >
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

#endif // DATY_HPP