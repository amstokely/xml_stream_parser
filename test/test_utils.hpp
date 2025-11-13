#ifndef XML_STREAM_PARSER_TEST_UTILS_HPP
#define XML_STREAM_PARSER_TEST_UTILS_HPP
#include <string>
#include "xml_stream_parser.hpp"

using namespace xml_stream_parser;

constexpr std::string operator""_s(const char* str, const std::size_t len) {
    return std::string{str, len};
}


auto get_xml_stream(const std::string& id, const pugi::xml_node& streams_node) {
    for (auto stream_xml : streams_node.children("immutable_stream")) {
        if (stream_xml.attribute("name").value() == id) {
            return PugiXmlAdapter{stream_xml};
        }
    }
    throw std::runtime_error("Stream not found: " + id);
}

#endif //XML_STREAM_PARSER_TEST_UTILS_HPP