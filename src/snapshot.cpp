#include "daty/snapshot.hpp"
#include "daty/serializer.hpp"
#include "file_manager.hpp"
#include <cstdint>

namespace daty {

Snapshot::Snapshot(const std::string& filePath) : filePath_(filePath) {}

void Snapshot::save(const HashTable<std::string, std::string>& table) {
    auto serializedData =
        Serializer::serialize(table.toVector(), table.getSequence() + table.getOperationCounter());

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

    std::uint64_t db_sequence;

    auto data = Serializer::deserialize(bytes, &db_sequence);

    HashTable<std::string, std::string> temporaryTable;
    temporaryTable.insert(data);

    temporaryTable.setSequence(db_sequence);

    table.swap(temporaryTable);
}

} // namespace daty