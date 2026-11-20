
#include <gtest/gtest.h>

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>

#include "file_manager.hpp"

using namespace daty;

class FileManagerTest : public ::testing::Test {
  protected:
    std::filesystem::path testDir =
        std::filesystem::temp_directory_path() / "daty_file_manager_tests";

    std::filesystem::path testFile = testDir / "test.bin";

    void SetUp() override {
        std::filesystem::create_directories(testDir);
    }

    void TearDown() override {
        std::filesystem::remove_all(testDir);
    }
};

// 1. exists()

TEST_F(FileManagerTest, ExistsReturnsFalseForNonexistentFile) {
    EXPECT_FALSE(FileManager::exists(testFile));
}

TEST_F(FileManagerTest, ExistsReturnsTrueForExistingFile) {
    std::ofstream file(testFile);
    file.close();

    EXPECT_TRUE(FileManager::exists(testFile));
}

// 2. fileSize()

TEST_F(FileManagerTest, FileSizeReturnsZeroForEmptyFile) {
    std::ofstream file(testFile);
    file.close();

    EXPECT_EQ(FileManager::fileSize(testFile), 0);
}

TEST_F(FileManagerTest, FileSizeReturnsCorrectSize) {
    std::vector<std::byte> data = {std::byte{0x10}, std::byte{0x20}, std::byte{0x30},
                                   std::byte{0x40}};

    FileManager::write(testFile, data);

    EXPECT_EQ(FileManager::fileSize(testFile), data.size());
}

// 3. write() and read()

TEST_F(FileManagerTest, WriteAndReadBinaryData) {
    std::vector<std::byte> data = {std::byte{0x00}, std::byte{0x10}, std::byte{0xFF},
                                   std::byte{0x80}, std::byte{0x00}};

    FileManager::write(testFile, data);

    auto result = FileManager::read(testFile);

    EXPECT_EQ(result, data);
}

TEST_F(FileManagerTest, WriteEmptyData) {
    std::vector<std::byte> data;

    FileManager::write(testFile, data);

    EXPECT_TRUE(FileManager::exists(testFile));
    EXPECT_TRUE(FileManager::read(testFile).empty());
    EXPECT_EQ(FileManager::fileSize(testFile), 0);
}

TEST_F(FileManagerTest, WriteOverwritesExistingFile) {
    std::vector<std::byte> original = {std::byte{0x10}, std::byte{0x20}, std::byte{0x30}};

    std::vector<std::byte> replacement = {std::byte{0xAA}, std::byte{0xBB}};

    FileManager::write(testFile, original);
    FileManager::write(testFile, replacement);

    EXPECT_EQ(FileManager::read(testFile), replacement);
}

// 4. append()

TEST_F(FileManagerTest, AppendAddsDataToExistingFile) {
    std::vector<std::byte> original = {std::byte{0x10}, std::byte{0x20}};

    std::vector<std::byte> additional = {std::byte{0x30}, std::byte{0x40}};

    FileManager::write(testFile, original);
    FileManager::append(testFile, additional);

    std::vector<std::byte> expected = {std::byte{0x10}, std::byte{0x20}, std::byte{0x30},
                                       std::byte{0x40}};

    EXPECT_EQ(FileManager::read(testFile), expected);
}

TEST_F(FileManagerTest, AppendCreatesFileIfItDoesNotExist) {
    std::vector<std::byte> data = {std::byte{0xAA}, std::byte{0xBB}};

    FileManager::append(testFile, data);

    EXPECT_TRUE(FileManager::exists(testFile));
    EXPECT_EQ(FileManager::read(testFile), data);
}

TEST_F(FileManagerTest, AppendEmptyDataDoesNotChangeFile) {
    std::vector<std::byte> original = {std::byte{0x10}, std::byte{0x20}};

    std::vector<std::byte> empty;

    FileManager::write(testFile, original);
    FileManager::append(testFile, empty);

    EXPECT_EQ(FileManager::read(testFile), original);
}

// 5. truncate()

TEST_F(FileManagerTest, TruncateClearsFileContents) {
    std::vector<std::byte> data = {std::byte{0x10}, std::byte{0x20}, std::byte{0x30}};

    FileManager::write(testFile, data);
    FileManager::truncate(testFile);

    EXPECT_TRUE(FileManager::exists(testFile));
    EXPECT_EQ(FileManager::fileSize(testFile), 0);
    EXPECT_TRUE(FileManager::read(testFile).empty());
}

// 6. Error handling

TEST_F(FileManagerTest, ReadThrowsForNonexistentFile) {
    EXPECT_THROW(FileManager::read(testFile), std::runtime_error);
}

TEST_F(FileManagerTest, FileSizeThrowsForNonexistentFile) {
    EXPECT_THROW(FileManager::fileSize(testFile), std::runtime_error);
}
