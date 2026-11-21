#include "daty/snapshot.hpp"
#include "daty/serializer.hpp"
#include "file_manager.hpp"

namespace daty {

Snapshot::Snapshot(const std::string& filePath) : filePath_(filePath) {}

void Snapshot::save(const HashTable<std::string, std::string>& table) {
    auto serializedData = Serializer::serialize(table.toVector());

    auto tempPath = filePath_;
    tempPath += ".tmp";

    try {
        FileManager::write(tempPath, serializedData);
        FileManager::rename(tempPath, filePath_.filename().string());
    } catch (...) {
        try {
            FileManager::clean(tempPath);
        } catch (...) {
            // Preserve the original exception.
        }
        throw;
    }
}

void Snapshot::load(HashTable<std::string, std::string>& table) {
    auto bytes = FileManager::read(filePath_);

    auto data = Serializer::deserialize(bytes);

    HashTable<std::string, std::string> temporaryTable;
    temporaryTable.insert(data);

    table.swap(temporaryTable);
}

} // namespace daty