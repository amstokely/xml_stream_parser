#pragma once
#ifndef XML_STREAM_PARSER_MOCK_XML_FILE_SYSTEM_HPP
#define XML_STREAM_PARSER_MOCK_XML_FILE_SYSTEM_HPP

#include "filesystem.hpp"

namespace xml_stream_parser::test {

class MockFileSystem final : public IXmlFileSystem {
public:
    bool exists_ret{false};
    bool create_success{true};
    bool writable{true};

    [[nodiscard]] bool exists(const std::string&) const noexcept override {
        return exists_ret;
    }

    bool create_directories(const std::string&) noexcept override {
        return create_success;
    }

    [[nodiscard]] bool can_write(const std::string&) const noexcept override {
        return writable;
    }
};

} // namespace xml_stream_parser::test

#endif // XML_STREAM_PARSER_MOCK_XML_FILE_SYSTEM_HPP
