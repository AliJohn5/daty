#pragma once

#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>
#include <stdexcept>


namespace daty {

template <
        typename Key,
        typename Value,
        typename Hash = std::hash<Key>
    >
class HashTable {

private:

    struct Node {
        std::pair<const Key, Value> data;
        std::unique_ptr<Node> next;

        Node(const Key& key, const Value& value)
            : data(key, value), next(nullptr) {}
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
                    Hash{}(node->data.first) % new_capacity;

                node->next = std::move(new_buckets[index]);

                new_buckets[index] = std::move(node);
            }
        }

        buckets_ = std::move(new_buckets);
    }

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

            if (current->data.first == key) {
                current->data.second = value;
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

            if (current->data.first == key) {
                return current->data.second;
            }

            current = current->next.get();
        }

        return std::nullopt;
    }

    bool contains(const Key& key) const {

        std::size_t index = hash(key);

        const Node* current = buckets_[index].get();

        while (current != nullptr) {

            if (current->data.first== key) {
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

            if ((*current)->data.first == key) {

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


    Value& at(const Key& key) {
        std::size_t index = hash(key);

        auto* current = buckets_[index].get();

        while (current != nullptr) {
            if (current->data.first== key) {
                return current->data.second;
            }

            current = current->next.get();
        }

        throw std::out_of_range("Key not found");
    }

    const Value& at(const Key& key) const {
        std::size_t index = hash(key);
        
        auto* current = buckets_[index].get();
        
        while (current != nullptr) {
            if (current->data.first== key) {
                return current->data.second;
            }
        
            current = current->next.get();
        }
    
        throw std::out_of_range("Key not found");
    }




    class Iterator {
    private:
        Node* current_;
        HashTable* table_;
        std::size_t bucket_index_;
        
    public:
        Iterator(
            Node* current,
            HashTable* table,
            std::size_t bucket_index
        )
            : current_(current),
              table_(table),
              bucket_index_(bucket_index) {}

            
        std::pair<const Key, Value>& operator*() const {
            return current_->data;
        }

        std::pair<const Key, Value>* operator->() const {
            return &current_->data;
        }

        bool operator==(const Iterator& other) const {
            return current_ == other.current_ &&
                   table_ == other.table_;
        }

        bool operator!=(const Iterator& other) const {
            return !(*this == other);
        }


        Iterator& operator++() {
            if (current_->next != nullptr) {
                current_ = current_->next.get();
                return *this;
            }
        
            ++bucket_index_;
        
            while (
                bucket_index_ < table_->buckets_.size() &&
                table_->buckets_[bucket_index_] == nullptr
            ) {
                ++bucket_index_;
            }
        
            if (bucket_index_ < table_->buckets_.size()) {
                current_ = table_->buckets_[bucket_index_].get();
            } else {
                current_ = nullptr;
            }
        
            return *this;
        }

        Iterator operator++(int) {
            Iterator previous = *this;
            ++(*this);
            return previous;
        }
    };


    Iterator begin() {
        for (std::size_t i = 0; i < buckets_.size(); ++i) {
            if (buckets_[i] != nullptr) {
                return Iterator(buckets_[i].get(), this, i);
            }
        }

        return end();
    }

    Iterator end() {
        return Iterator(nullptr, this, buckets_.size());
    }


   
};




} // namespace kvstore
