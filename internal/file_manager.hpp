#pragma once

#include <cstddef>
#include <filesystem>
#include <ios>
#include <span>
#include <vector>

namespace daty {

class FileManager {

  public:
    static bool exists(const std::filesystem::path& path);

    static std::size_t fileSize(const std::filesystem::path& path);

    static void rename(const std::filesystem::path& path, const std::string& newName);

    static std::vector<std::byte> read(const std::filesystem::path& path);

    static void write(const std::filesystem::path& path, std::span<const std::byte> data);

    static void append(const std::filesystem::path& path, std::span<const std::byte> data);

    static void write(const std::filesystem::path& path, std::span<const std::byte> data,
                      std::ios_base::openmode mode);

    static void truncate(const std::filesystem::path& path);

    static void clean(const std::filesystem::path& path);
};

} // namespace daty