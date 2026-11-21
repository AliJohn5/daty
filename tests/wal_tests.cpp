
#include "daty/wal.hpp"

#include "file_manager.hpp"

#include <gtest/gtest.h>

#include <cstddef>
#include <filesystem>
#include <span>
#include <string>
#include <vector>

namespace daty {

class WalTest : public ::testing::Test {

  protected:
    std::filesystem::path testFile = std::filesystem::temp_directory_path() / "daty_test.wal";

    Wal wal{testFile};

    void SetUp() override {
        std::filesystem::remove(testFile);
    }

    void TearDown() override {
        std::filesystem::remove(testFile);
    }

    static std::vector<std::byte> makePayload(const std::string& value) {
        std::vector<std::byte> payload;

        for (unsigned char c : value) {
            payload.push_back(static_cast<std::byte>(c));
        }

        return payload;
    }

    void writeRawData(const std::vector<std::byte>& data) {
        FileManager::write(testFile, data);
    }

    std::vector<std::byte> readRawData() {
        return FileManager::read(testFile);
    }
};

// --------------------------------------------------
// Constructor
// --------------------------------------------------

TEST_F(WalTest, ConstructorDoesNotCreateFile) {
    EXPECT_FALSE(FileManager::exists(testFile));
}

// --------------------------------------------------
// Append
// --------------------------------------------------

TEST_F(WalTest, AppendCreatesWalFile) {
    const auto payload = makePayload("hello");

    wal.append(WalOperation::PUT, payload);

    EXPECT_TRUE(FileManager::exists(testFile));
}

TEST_F(WalTest, AppendWritesRecord) {
    const auto payload = makePayload("hello");

    wal.append(WalOperation::PUT, payload);

    EXPECT_GT(FileManager::fileSize(testFile), 0);
}

TEST_F(WalTest, AppendMultipleRecords) {
    const auto payload1 = makePayload("first");
    const auto payload2 = makePayload("second");

    wal.append(WalOperation::PUT, payload1);
    wal.append(WalOperation::DELETE, payload2);

    const auto records = wal.readAll();

    ASSERT_EQ(records.size(), 2);

    EXPECT_EQ(records[0].operation, WalOperation::PUT);
    EXPECT_EQ(records[0].payload, payload1);

    EXPECT_EQ(records[1].operation, WalOperation::DELETE);
    EXPECT_EQ(records[1].payload, payload2);
}

TEST_F(WalTest, AppendEmptyPayload) {
    const std::vector<std::byte> payload;

    EXPECT_NO_THROW(wal.append(WalOperation::PUT, payload));

    const auto records = wal.readAll();

    ASSERT_EQ(records.size(), 1);
    EXPECT_TRUE(records[0].payload.empty());
}

TEST_F(WalTest, AppendInvalidOperationThrows) {
    const auto payload = makePayload("hello");

    EXPECT_THROW(wal.append(static_cast<WalOperation>(255), payload), std::invalid_argument);
}

// --------------------------------------------------
// Read
// --------------------------------------------------

TEST_F(WalTest, ReadNonexistentFileReturnsEmptyVector) {
    const auto records = wal.readAll();

    EXPECT_TRUE(records.empty());
}

TEST_F(WalTest, ReadEmptyFileReturnsEmptyVector) {
    FileManager::write(testFile, std::vector<std::byte>{});

    const auto records = wal.readAll();

    EXPECT_TRUE(records.empty());
}

TEST_F(WalTest, ReadPreservesRecordOrder) {
    const auto payload1 = makePayload("one");
    const auto payload2 = makePayload("two");
    const auto payload3 = makePayload("three");

    wal.append(WalOperation::PUT, payload1);
    wal.append(WalOperation::DELETE, payload2);
    wal.append(WalOperation::PUT, payload3);

    const auto records = wal.readAll();

    ASSERT_EQ(records.size(), 3);

    EXPECT_EQ(records[0].payload, payload1);
    EXPECT_EQ(records[1].payload, payload2);
    EXPECT_EQ(records[2].payload, payload3);
}

TEST_F(WalTest, ReadPreservesEmptyPayload) {
    const std::vector<std::byte> payload;

    wal.append(WalOperation::DELETE, payload);

    const auto records = wal.readAll();

    ASSERT_EQ(records.size(), 1);
    EXPECT_TRUE(records[0].payload.empty());
}

// --------------------------------------------------
// Corrupted records
// --------------------------------------------------

TEST_F(WalTest, ReadThrowsForInvalidMagic) {
    wal.append(WalOperation::PUT, makePayload("hello"));

    auto data = readRawData();

    data[0] = std::byte{0xFF};

    writeRawData(data);

    EXPECT_THROW(wal.readAll(), std::runtime_error);
}

TEST_F(WalTest, ReadThrowsForUnsupportedVersion) {
    wal.append(WalOperation::PUT, makePayload("hello"));

    auto data = readRawData();

    data[4] = std::byte{0xFF};

    writeRawData(data);

    EXPECT_THROW(wal.readAll(), std::runtime_error);
}

TEST_F(WalTest, ReadThrowsForInvalidOperation) {
    wal.append(WalOperation::PUT, makePayload("hello"));

    auto data = readRawData();

    data[5] = std::byte{0xFF};

    writeRawData(data);

    EXPECT_THROW(wal.readAll(), std::runtime_error);
}

TEST_F(WalTest, ReadThrowsForChecksumMismatch) {
    wal.append(WalOperation::PUT, makePayload("hello"));

    auto data = readRawData();

    data.back() ^= std::byte{0x01};

    writeRawData(data);

    EXPECT_THROW(wal.readAll(), std::runtime_error);
}

// --------------------------------------------------
// Incomplete records
// --------------------------------------------------

TEST_F(WalTest, ReadIgnoresIncompleteHeader) {
    const std::vector<std::byte> data = {std::byte{'D'}, std::byte{'A'}, std::byte{'T'},
                                         std::byte{'Y'}, std::byte{1}};

    writeRawData(data);

    EXPECT_NO_THROW({
        const auto records = wal.readAll();
        EXPECT_TRUE(records.empty());
    });
}

TEST_F(WalTest, ReadIgnoresIncompletePayload) {
    wal.append(WalOperation::PUT, makePayload("hello"));

    auto data = readRawData();

    // Remove two bytes from the end of the record.
    data.resize(data.size() - 2);

    writeRawData(data);

    EXPECT_NO_THROW({
        const auto records = wal.readAll();
        EXPECT_TRUE(records.empty());
    });
}

TEST_F(WalTest, ReadValidRecordsBeforeIncompleteRecord) {
    const auto payload = makePayload("valid");

    wal.append(WalOperation::PUT, payload);

    auto data = readRawData();

    // Add an incomplete header after the valid record.
    data.push_back(std::byte{'D'});
    data.push_back(std::byte{'A'});
    data.push_back(std::byte{'T'});

    writeRawData(data);

    const auto records = wal.readAll();

    ASSERT_EQ(records.size(), 1);
    EXPECT_EQ(records[0].payload, payload);
}

TEST_F(WalTest, ReadThrowsWhenCorruptedRecordFollowsValidRecord) {
    wal.append(WalOperation::PUT, makePayload("valid"));
    wal.append(WalOperation::PUT, makePayload("corrupted"));

    auto data = readRawData();

    constexpr std::size_t HEADER_SIZE = 14;

    // First record: 14 + 5 + 4 bytes.
    constexpr std::size_t FIRST_RECORD_SIZE = 23;

    // Corrupt the second record's payload.
    data[FIRST_RECORD_SIZE + HEADER_SIZE] ^= std::byte{0x01};

    writeRawData(data);

    EXPECT_THROW(wal.readAll(), std::runtime_error);
}

TEST_F(WalTest, ReadThrowsWhenValidRecordFollowsCorruptedRecord) {
    wal.append(WalOperation::PUT, makePayload("corrupted"));
    wal.append(WalOperation::PUT, makePayload("valid"));

    auto data = readRawData();

    constexpr std::size_t HEADER_SIZE = 14;

    // Corrupt the first record's payload.
    data[HEADER_SIZE] ^= std::byte{0x01};

    writeRawData(data);

    EXPECT_THROW(wal.readAll(), std::runtime_error);
}
} // namespace daty
