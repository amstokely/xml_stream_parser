#pragma once
#ifndef XML_STREAM_PARSER_FILESYSTEM_HPP
#define XML_STREAM_PARSER_FILESYSTEM_HPP

#include <string>
#include <filesystem>
#include <system_error>

namespace xml_stream_parser {

namespace fs = std::filesystem;

// -----------------------------------------------------------------------------
// Interface: IXmlFileSystem
// -----------------------------------------------------------------------------
struct IXmlFileSystem {
    virtual ~IXmlFileSystem() = default;

    [[nodiscard]] virtual bool exists(const std::string& path) const noexcept = 0;
    [[nodiscard]] virtual bool can_write(const std::string& path) const noexcept = 0;
    virtual bool create_directories(const std::string& path) noexcept = 0;
};

// -----------------------------------------------------------------------------
// Implementation: XmlFileSystem
// -----------------------------------------------------------------------------
class XmlFileSystem final : public IXmlFileSystem {
public:
    [[nodiscard]] bool exists(const std::string& path) const noexcept override {
        std::error_code ec;
        return fs::exists(path, ec) && !ec;
    }

    bool create_directories(const std::string& path) noexcept override {
        std::error_code ec;
        fs::create_directories(path, ec);
        return !ec;
    }

    [[nodiscard]] bool can_write(const std::string& path) const noexcept override {
        std::error_code ec;
        const auto st = fs::status(path, ec);
        if (ec) return false;

        const auto perms = st.permissions();
        constexpr auto write_mask = fs::perms::owner_write |
                                    fs::perms::group_write |
                                    fs::perms::others_write;
        return (perms & write_mask) != fs::perms::none;
    }
};

} // namespace xml_stream_parser

#endif // XML_STREAM_PARSER_FILESYSTEM_HPP
