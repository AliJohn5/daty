#include "file_manager.hpp"
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <limits>
#include <stdexcept>

namespace daty {

bool FileManager::exists(const std::filesystem::path& path) {
    return std::filesystem::exists(path);
}

std::size_t FileManager::fileSize(const std::filesystem::path& path) {
    return static_cast<std::size_t>(std::filesystem::file_size(path));
}

std::vector<std::byte> FileManager::read(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);

    if (!file) {
        throw std::runtime_error("Failed to open file");
    }

    file.seekg(0, std::ios::end);

    if (!file) {
        throw std::runtime_error("Failed to determine file size");
    }

    const auto fileSize = file.tellg();

    if (fileSize < 0) {
        throw std::runtime_error("Invalid file size");
    }

    std::vector<std::byte> buffer(static_cast<std::size_t>(fileSize));

    file.seekg(0, std::ios::beg);

    if (!file) {
        throw std::runtime_error("Failed to seek to beginning");
    }

    if (!buffer.empty()) {
        file.read(reinterpret_cast<char*>(buffer.data()),
                  static_cast<std::streamsize>(buffer.size()));

        if (!file) {
            throw std::runtime_error("Failed to read file");
        }
    }

    return buffer;
}

void FileManager::write(const std::filesystem::path& path, std::span<const std::byte> data,
                        std::ios_base::openmode mode) {
    if (data.size() > static_cast<std::size_t>(std::numeric_limits<std::streamsize>::max())) {
        throw std::length_error("Data is too large to write");
    }

    std::ofstream file(path, mode);

    if (!file) {
        throw std::runtime_error("Failed to open file");
    }

    if (!data.empty()) {
        file.write(reinterpret_cast<const char*>(data.data()),
                   static_cast<std::streamsize>(data.size()));

        if (!file) {
            throw std::runtime_error("Failed to write file");
        }
    }

    file.flush();

    if (!file) {
        throw std::runtime_error("Failed to flush file");
    }
}

void FileManager::write(const std::filesystem::path& path, std::span<const std::byte> data) {
    return write(path, data, std::ios::binary | std::ios::trunc);
}

void FileManager::append(const std::filesystem::path& path, std::span<const std::byte> data) {
    return write(path, data, std::ios::binary | std::ios::app);
}

void FileManager::truncate(const std::filesystem::path& path) {

    std::ofstream file(path, std::ios::binary | std::ios::trunc);

    if (!file) {
        throw std::runtime_error("Failed to open file");
    }

    file.close();

    if (!file) {
        throw std::runtime_error("Failed to close file");
    }
}

void FileManager::rename(const std::filesystem::path& path, const std::string& newName) {
    if (!std::filesystem::exists(path)) {
        throw std::runtime_error("File does not exist: " + path.string());
    }

    if (newName.empty()) {
        throw std::invalid_argument("New file name cannot be empty");
    }

    std::filesystem::path newPath = path.parent_path() / newName;

    std::filesystem::rename(path, newPath);
}

void FileManager::clean(const std::filesystem::path& path) {
    std::error_code error;

    std::filesystem::remove(path, error);

    if (error) {
        throw std::runtime_error("Failed to clean file: " + path.string() + " (" + error.message() +
                                 ")");
    }
}

} // namespace daty