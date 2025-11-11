#ifndef XML_STREAM_PARSER_FILESYSTEM_HPP
#define XML_STREAM_PARSER_FILESYSTEM_HPP
#pragma once

#include <string>
#include <filesystem>
#include <system_error>

namespace xml_stream_parser {

    namespace fs = std::filesystem;

    struct IXmlFileSystem {
        virtual ~IXmlFileSystem() = default;

        [[nodiscard]] virtual bool exists(const std::string &path) const = 0;
        virtual bool create_directories(const std::string &path) = 0;
        [[nodiscard]] virtual bool can_write(const std::string &path) const = 0;
    };

    struct XmlFileSystem final : IXmlFileSystem {
        [[nodiscard]] bool exists(const std::string &path) const override {
            return fs::exists(path);
        }

        bool create_directories(const std::string &path) override {
            std::error_code ec;
            fs::create_directories(path, ec);
            return !ec;
        }

        [[nodiscard]] bool can_write(const std::string &path) const override {
            std::error_code ec;
            auto st = fs::status(path, ec);
            if (ec) return false;

            auto perms = st.permissions();
            return (perms & (fs::perms::owner_write |
                             fs::perms::group_write |
                             fs::perms::others_write)) != fs::perms::none;
        }
    };

} // namespace xml_stream_parser
#endif //XML_STREAM_PARSER_FILESYSTEM_HPP