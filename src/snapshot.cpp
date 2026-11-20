#include "daty/snapshot.hpp"
#include "daty/serializer.hpp"
#include "file_manager.hpp"

namespace daty {

Snapshot::Snapshot(const std::string& filePath) : filePath_(filePath) {}

void Snapshot::save(const HashTable<std::string, std::string>& table) {
    auto serializedData = Serializer::serialize(table.toVector());
    FileManager::write(filePath_, serializedData);
}

void Snapshot::load(HashTable<std::string, std::string>& table) {
    auto bytes = FileManager::read(filePath_);
    auto data = Serializer::deserialize(bytes);
    table.clear();
    table.insert(data);
}

} // namespace daty