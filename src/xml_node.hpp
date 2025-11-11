#ifndef XML_STREAM_PARSER_XML_NODE_HPP
#define XML_STREAM_PARSER_XML_NODE_HPP
#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <pugixml.hpp>

namespace xml_stream_parser {

    template<typename T>
    concept XmlNode = requires(const T &node, std::string_view key, std::string_view tag) {
        { node.get_attribute(key) } -> std::convertible_to<std::string>;
        { node.has_attribute(key) } -> std::convertible_to<bool>;
        { node.children(tag) }      -> std::same_as<std::vector<T>>;
    };

    struct PugiXmlAdapter {
        pugi::xml_node node;

        explicit PugiXmlAdapter(pugi::xml_node n) : node(n) {}

        std::string get_attribute(std::string_view key) const {
            if (auto a = node.attribute(std::string(key).c_str()))
                return a.value();
            return {};
        }

        bool has_attribute(std::string_view key) const {
            return node.attribute(std::string(key).c_str());
        }

        std::vector<PugiXmlAdapter> children(std::string_view tag) const {
            std::vector<PugiXmlAdapter> out;
            for (auto c : node.children(std::string(tag).c_str()))
                out.emplace_back(c);
            return out;
        }
    };

} // namespace xml_stream_parser

#endif //XML_STREAM_PARSER_XML_NODE_HPP