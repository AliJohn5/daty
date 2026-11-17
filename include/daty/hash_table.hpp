
#ifndef DATY_HASH_TABLE_HPP
#define DATY_HASH_TABLE_HPP

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace daty {

template <
        typename Key,
        typename Value,
        typename Hash = std::hash<Key>
    >
class HashTable {
public:

    explicit HashTable(std::size_t initial_capacity = 16)
        : buckets_(initial_capacity) {

        if (initial_capacity == 0) {
            throw std::invalid_argument(
                "Hash table capacity must be greater than zero"
            );
        }
    }

    // Disable copying for now.
    HashTable(const HashTable&) = delete;
    HashTable& operator=(const HashTable&) = delete;

    void insert(const Key& key, const Value& value) {

        std::size_t index = hash(key);

        Node* current = buckets_[index].get();

        // Check if the key already exists in the bucket
        while (current != nullptr) {

            if (current->key == key) {
                current->value = value;
                return;
            }

            current = current->next.get();
        }

        // Check if we need to rehash before inserting
        if (static_cast<float>(size_ + 1) >
            buckets_.size() * MAX_LOAD_FACTOR) {

            rehash(buckets_.size() * 2);

            index = hash(key);
        }

        // Insert the new key-value pair at the beginning of the bucket
        auto node = std::make_unique<Node>(key, value);

        node->next = std::move(buckets_[index]);

        buckets_[index] = std::move(node);

        ++size_;
    }

    std::optional<Value> get(const Key& key) const {

        std::size_t index = hash(key);

        const Node* current = buckets_[index].get();

        while (current != nullptr) {

            if (current->key == key) {
                return current->value;
            }

            current = current->next.get();
        }

        return std::nullopt;
    }

    bool contains(const Key& key) const {

        std::size_t index = hash(key);

        const Node* current = buckets_[index].get();

        while (current != nullptr) {

            if (current->key == key) {
                return true;
            }

            current = current->next.get();
        }

        return false;
    }

    bool remove(const Key& key) {

        std::size_t index = hash(key);

        auto* current = &buckets_[index];

        while (*current != nullptr) {

            if ((*current)->key == key) {

                *current = std::move((*current)->next);

                --size_;

                return true;
            }

            current = &((*current)->next);
        }

        return false;
    }

    std::size_t size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }

    std::size_t bucket_count() const {
        return buckets_.size();
    }

    void clear() {

        for (auto& bucket : buckets_) {
            bucket.reset();
        }

        size_ = 0;
    }

private:

    struct Node {
        Key key;
        Value value;
        std::unique_ptr<Node> next;

        Node(const Key& key, const Value& value)
            : key(key), value(value) {}
    };

    std::vector<std::unique_ptr<Node>> buckets_;

    std::size_t size_ = 0;

    static constexpr float MAX_LOAD_FACTOR = 0.75f;

    std::size_t hash(const Key& key) const {
        return Hash{}(key) % buckets_.size();
    }

    void rehash(std::size_t new_capacity) {

        std::vector<std::unique_ptr<Node>> new_buckets(
            new_capacity
        );

        for (auto& bucket : buckets_) {

            while (bucket != nullptr) {

                auto node = std::move(bucket);

                bucket = std::move(node->next);

                std::size_t index =
                    Hash{}(node->key) % new_capacity;

                node->next = std::move(new_buckets[index]);

                new_buckets[index] = std::move(node);
            }
        }

        buckets_ = std::move(new_buckets);
    }
};

} // namespace kvstore

#endif
