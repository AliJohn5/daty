
#include "daty/wal.hpp"

#include "file_manager.hpp"

#include <array>
#include <limits>
#include <stdexcept>
#include <utility>

namespace daty {

namespace {

constexpr std::array<std::byte, 4> WAL_MAGIC = {std::byte{'D'}, std::byte{'A'}, std::byte{'T'},
                                                std::byte{'Y'}};

constexpr std::uint8_t WAL_VERSION = 1;

constexpr std::size_t MAGIC_SIZE = 4;
constexpr std::size_t VERSION_SIZE = 1;
constexpr std::size_t OPERATION_SIZE = 1;
constexpr std::size_t LENGTH_SIZE = 8;
constexpr std::size_t CHECKSUM_SIZE = 4;

constexpr std::size_t HEADER_SIZE = MAGIC_SIZE + VERSION_SIZE + OPERATION_SIZE + LENGTH_SIZE;

// --------------------------------------------------
// Integer encoding
// --------------------------------------------------

void writeUint64(std::vector<std::byte>& buffer, std::uint64_t value) {
    for (std::size_t i = 0; i < LENGTH_SIZE; ++i) {
        buffer.push_back(static_cast<std::byte>((value >> (i * 8)) & 0xFF));
    }
}

std::uint64_t readUint64(std::span<const std::byte> data, std::size_t offset) {
    if (offset > data.size() || data.size() - offset < LENGTH_SIZE) {
        throw std::runtime_error("Incomplete WAL integer");
    }

    std::uint64_t value = 0;

    for (std::size_t i = 0; i < LENGTH_SIZE; ++i) {
        value |= static_cast<std::uint64_t>(std::to_integer<std::uint8_t>(data[offset + i]))
                 << (i * 8);
    }

    return value;
}

void writeUint32(std::vector<std::byte>& buffer, std::uint32_t value) {
    for (std::size_t i = 0; i < 4; ++i) {
        buffer.push_back(static_cast<std::byte>((value >> (i * 8)) & 0xFF));
    }
}

std::uint32_t readUint32(std::span<const std::byte> data, std::size_t offset) {
    if (offset > data.size() || data.size() - offset < CHECKSUM_SIZE) {
        throw std::runtime_error("Incomplete WAL checksum");
    }

    std::uint32_t value = 0;

    for (std::size_t i = 0; i < 4; ++i) {
        value |= static_cast<std::uint32_t>(std::to_integer<std::uint8_t>(data[offset + i]))
                 << (i * 8);
    }

    return value;
}

// --------------------------------------------------
// CRC32
// --------------------------------------------------

std::uint32_t calculateCRC32(std::span<const std::byte> data) {
    std::uint32_t crc = 0xFFFFFFFF;

    for (const auto byte : data) {
        crc ^= std::to_integer<std::uint8_t>(byte);

        for (int i = 0; i < 8; ++i) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
    }

    return crc ^ 0xFFFFFFFF;
}

// --------------------------------------------------
// Record encoding
// --------------------------------------------------

std::vector<std::byte> encodeRecord(WalOperation operation, std::span<const std::byte> payload) {
    if (operation != WalOperation::PUT && operation != WalOperation::DELETE &&
        operation != WalOperation::CLEAR) {
        throw std::invalid_argument("Invalid WAL operation");
    }

    if (operation == WalOperation::CLEAR && !payload.empty()) {
        throw std::invalid_argument("CLEAR operation cannot have a payload");
    }

    std::vector<std::byte> record;

    if (payload.size() > std::numeric_limits<std::uint64_t>::max()) {
        throw std::length_error("WAL payload is too large");
    }

    record.reserve(HEADER_SIZE + payload.size() + CHECKSUM_SIZE);

    // Magic
    record.insert(record.end(), WAL_MAGIC.begin(), WAL_MAGIC.end());

    // Version
    record.push_back(static_cast<std::byte>(WAL_VERSION));

    // Operation
    record.push_back(static_cast<std::byte>(operation));

    // Payload length
    writeUint64(record, static_cast<std::uint64_t>(payload.size()));

    // Payload
    record.insert(record.end(), payload.begin(), payload.end());

    // Checksum over header + payload
    const std::uint32_t checksum = calculateCRC32(record);

    writeUint32(record, checksum);

    return record;
}

// --------------------------------------------------
// Record decoding
// --------------------------------------------------

WalRecord decodeRecord(std::span<const std::byte> data, std::size_t offset,
                       std::size_t payloadLength) {
    const auto operationValue = std::to_integer<std::uint8_t>(data[offset + 5]);

    const auto operation = static_cast<WalOperation>(operationValue);

    if (operation != WalOperation::PUT && operation != WalOperation::DELETE &&
        operation != WalOperation::CLEAR) {
        throw std::runtime_error("Invalid WAL operation");
    }

    const std::size_t payloadOffset = offset + HEADER_SIZE;

    std::vector<std::byte> payload(data.begin() + payloadOffset,
                                   data.begin() + payloadOffset + payloadLength);

    return WalRecord{operation, std::move(payload)};
}

} // namespace

// --------------------------------------------------
// Wal
// --------------------------------------------------

Wal::Wal(std::filesystem::path path) : path_(std::move(path)) {}

// --------------------------------------------------
// Append
// --------------------------------------------------

void Wal::append(WalOperation operation, std::span<const std::byte> payload) {
    const auto record = encodeRecord(operation, payload);

    FileManager::append(path_, record);
}

// --------------------------------------------------
// Read all records
// --------------------------------------------------

std::vector<WalRecord> Wal::readAll() const {
    if (!FileManager::exists(path_)) {
        return {};
    }

    const auto data = FileManager::read(path_);

    std::vector<WalRecord> records;

    std::size_t offset = 0;

    while (offset < data.size()) {

        // Incomplete header: ignore trailing bytes.
        if (data.size() - offset < HEADER_SIZE) {
            break;
        }

        // Validate magic.
        for (std::size_t i = 0; i < MAGIC_SIZE; ++i) {
            if (data[offset + i] != WAL_MAGIC[i]) {
                throw std::runtime_error("Invalid WAL magic");
            }
        }

        // Validate version.
        const auto version = std::to_integer<std::uint8_t>(data[offset + 4]);

        if (version != WAL_VERSION) {
            throw std::runtime_error("Unsupported WAL version");
        }

        // Validate operation.
        const auto operation = std::to_integer<std::uint8_t>(data[offset + 5]);

        if (operation != static_cast<std::uint8_t>(WalOperation::PUT) &&
            operation != static_cast<std::uint8_t>(WalOperation::DELETE) &&
            operation != static_cast<std::uint8_t>(WalOperation::CLEAR)) {
            throw std::runtime_error("Invalid WAL operation");
        }

        // Read payload length.
        const std::uint64_t payloadLength = readUint64(data, offset + 6);

        if (operation == static_cast<std::uint8_t>(WalOperation::CLEAR) && payloadLength != 0) {
            throw std::runtime_error("Invalid CLEAR record");
        }

        const std::size_t remaining = data.size() - offset - HEADER_SIZE;

        // Incomplete payload or checksum.
        if (payloadLength > remaining ||
            remaining - static_cast<std::size_t>(payloadLength) < CHECKSUM_SIZE) {
            break;
        }

        const std::size_t payloadSize = static_cast<std::size_t>(payloadLength);

        const std::size_t recordSize = HEADER_SIZE + payloadSize + CHECKSUM_SIZE;

        // Read stored checksum.
        const std::size_t checksumOffset = offset + HEADER_SIZE + payloadSize;

        const std::uint32_t storedChecksum = readUint32(data, checksumOffset);

        // Calculate checksum over header + payload.
        const auto recordData =
            std::span<const std::byte>(data.data() + offset, HEADER_SIZE + payloadSize);

        const std::uint32_t calculatedChecksum = calculateCRC32(recordData);

        if (storedChecksum != calculatedChecksum) {
            throw std::runtime_error("WAL checksum mismatch");
        }

        // Decode record.
        records.push_back(decodeRecord(data, offset, payloadSize));

        offset += recordSize;
    }

    return records;
}

} // namespace daty
