#ifndef XML_STREAM_PARSER_MOCK_XML_FILE_SYSTEM_HPP
#define XML_STREAM_PARSER_MOCK_XML_FILE_SYSTEM_HPP
#include "xml_stream_parser.hpp"

using namespace xml_stream_parser;
struct MockFileSystem : IXmlFileSystem {
    bool exists_ret = false;
    bool create_success = true;
    bool writable = true;

    bool exists(const std::string&) const override { return exists_ret; }
    bool create_directories(const std::string&) override { return create_success; }
    bool can_write(const std::string&) const override { return writable; }
};



#endif //XML_STREAM_PARSER_MOCK_XML_FILE_SYSTEM_HPP