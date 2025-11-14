#pragma once
#ifndef XML_STREAM_PARSER_FILESYSTEM_HPP
#define XML_STREAM_PARSER_FILESYSTEM_HPP

#include <string>
#include <filesystem>
#include <system_error>

namespace xml_stream_parser {

namespace fs = std::filesystem;

/**
 * @defgroup filesystem_backend Filesystem Backend
 * @brief Abstractions and utilities for interacting with the host filesystem.
 * @{
 */

/**
 * @class IXmlFileSystem
 * @brief Interface for filesystem operations used by the XML stream parser.
 *
 * This abstraction isolates the parser from the concrete filesystem API
 * (`std::filesystem`), making it easy to inject mock behavior for unit tests.
 *
 * Required capabilities:
 *   - Existence checks
 *   - Directory creation
 *   - Write-permission checks
 *
 * Implementations should never throw exceptions. All methods must return
 * boolean success/failure indicators instead.
 */
struct IXmlFileSystem {
    virtual ~IXmlFileSystem() = default;

    /**
     * @brief Checks whether a path exists on the filesystem.
     * @param path Filesystem path.
     * @return True if the path exists; false otherwise.
     */
    [[nodiscard]] virtual bool exists(const std::string& path) const noexcept = 0;

    /**
     * @brief Checks whether the current user has write access to the path.
     * @param path Filesystem path.
     * @return True if the directory is writable; false otherwise.
     */
    [[nodiscard]] virtual bool can_write(const std::string& path) const noexcept = 0;

    /**
     * @brief Attempts to create the directory hierarchy for the given path.
     * @param path Directory path.
     * @return True on success; false if creation failed.
     */
    virtual bool create_directories(const std::string& path) noexcept = 0;
};

/**
 * @class XmlFileSystem
 * @brief Concrete `IXmlFileSystem` implementation using `std::filesystem`.
 *
 * This class provides filesystem behavior for production use. All methods use
 * `std::error_code` overloads to ensure **no exceptions** are thrown.
 */
class XmlFileSystem final : public IXmlFileSystem {
public:
    /**
     * @brief Checks whether a path exists on disk.
     *
     * Uses the error-code overload to avoid exceptions.
     */
    [[nodiscard]] bool exists(const std::string& path) const noexcept override {
        std::error_code ec;
        return fs::exists(path, ec) && !ec;
    }

    /**
     * @brief Recursively creates directories using `std::filesystem::create_directories`.
     *
     * @return True if creation succeeds or the directory already exists.
     */
    bool create_directories(const std::string& path) noexcept override {
        std::error_code ec;
        fs::create_directories(path, ec);
        return !ec;
    }

    /**
     * @brief Determines whether the directory at `path` is writable.
     *
     * Permission logic:
     *   - Inspects POSIX-style write bits for owner / group / others.
     *   - If permission bits cannot be retrieved, returns false.
     *
     * This is a conservative check: failure to confirm write access results in
     * a "not writable" outcome to protect against incorrect filesystem use.
     */
    [[nodiscard]] bool can_write(const std::string& path) const noexcept override {
        std::error_code ec;
        const auto st = fs::status(path, ec);
        if (ec) return false;

        const auto perms = st.permissions();
        constexpr auto write_mask =
            fs::perms::owner_write |
            fs::perms::group_write |
            fs::perms::others_write;

        return (perms & write_mask) != fs::perms::none;
    }
};

/** @} */ // end of filesystem_backend

} // namespace xml_stream_parser

#endif // XML_STREAM_PARSER_FILESYSTEM_HPP
