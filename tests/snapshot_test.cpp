
#include <gtest/gtest.h>

#include <algorithm>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "daty/hash_table.hpp"
#include "daty/snapshot.hpp"

using namespace daty;

class SnapshotTest : public ::testing::Test {
  protected:
    using Entry = std::pair<std::string, std::string>;
    using Entries = std::vector<Entry>;

    std::filesystem::path testFile =
        std::filesystem::temp_directory_path() / "daty_snapshot_test.bin";

    void SetUp() override {
        std::filesystem::remove(testFile);
    }

    void TearDown() override {
        std::filesystem::remove(testFile);
    }

    Entries sortedEntries(const HashTable<std::string, std::string>& table) {
        auto entries = table.toVector();
        std::sort(entries.begin(), entries.end());
        return entries;
    }
};

// Save and load multiple entries
TEST_F(SnapshotTest, SaveAndLoadMultipleEntries) {
    HashTable<std::string, std::string> original;

    original.insert("name", "Ali");
    original.insert("language", "C++");
    original.insert("project", "daty");

    Snapshot snapshot(testFile.string());

    snapshot.save(original);

    HashTable<std::string, std::string> restored;
    snapshot.load(restored);

    EXPECT_EQ(sortedEntries(restored), sortedEntries(original));
}

// Save and load an empty table
TEST_F(SnapshotTest, SaveAndLoadEmptyTable) {
    HashTable<std::string, std::string> original;

    Snapshot snapshot(testFile.string());

    snapshot.save(original);

    HashTable<std::string, std::string> restored;
    snapshot.load(restored);

    EXPECT_TRUE(restored.toVector().empty());
}

// Load into an existing table (merge behavior)
TEST_F(SnapshotTest, LoadReplacesExistingTable) {
    HashTable<std::string, std::string> original;

    original.insert("name", "Ali");
    original.insert("language", "C++");

    Snapshot snapshot(testFile.string());
    snapshot.save(original);

    HashTable<std::string, std::string> restored;

    restored.insert("city", "Latakia");
    restored.insert("language", "Python");

    snapshot.load(restored);

    Entries expected = {{"language", "C++"}, {"name", "Ali"}};

    std::sort(expected.begin(), expected.end());

    EXPECT_EQ(sortedEntries(restored), expected);
}
// Saving again should overwrite the previous snapshot
TEST_F(SnapshotTest, SaveOverwritesPreviousSnapshot) {
    HashTable<std::string, std::string> first;
    first.insert("version", "1");

    HashTable<std::string, std::string> second;
    second.insert("version", "2");
    second.insert("language", "C++");

    Snapshot snapshot(testFile.string());

    snapshot.save(first);
    snapshot.save(second);

    HashTable<std::string, std::string> restored;
    snapshot.load(restored);

    EXPECT_EQ(sortedEntries(restored), sortedEntries(second));
}

// Loading a nonexistent file should throw
TEST_F(SnapshotTest, LoadThrowsForNonexistentFile) {
    Snapshot snapshot(testFile.string());

    HashTable<std::string, std::string> table;

    EXPECT_THROW(snapshot.load(table), std::runtime_error);
}
