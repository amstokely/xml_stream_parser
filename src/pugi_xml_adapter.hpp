#pragma once
#ifndef XML_STREAM_PARSER_XML_NODE_HPP
#define XML_STREAM_PARSER_XML_NODE_HPP

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <pugixml.hpp>

namespace xml_stream_parser {

class PugiXmlAdapter {
public:
    explicit PugiXmlAdapter(pugi::xml_node n) noexcept : node_{n} {}

    [[nodiscard]] std::string get_attribute(std::string_view key) const noexcept {
        if (const auto attr = node_.attribute(std::string{key}.c_str()))
            return attr.value();
        return {};
    }

    [[nodiscard]] std::unordered_map<std::string, std::string> get_attributes() const {
        std::unordered_map<std::string, std::string> attrs;
        attrs.reserve(static_cast<size_t>(std::distance(node_.attributes_begin(),
                                                        node_.attributes_end())));
        for (const auto& attr : node_.attributes())
            attrs.emplace(attr.name(), attr.value());
        return attrs;
    }

    [[nodiscard]] bool has_attribute(std::string_view key) const noexcept {
        return node_.attribute(std::string{key}.c_str());
    }

    [[nodiscard]] std::vector<PugiXmlAdapter> children(std::string_view tag) const {
        std::vector<PugiXmlAdapter> result;
        for (const auto& child : node_.children(std::string{tag}.c_str()))
            result.emplace_back(child);
        return result;
    }

    [[nodiscard]] std::string name() const noexcept { return node_.name(); }

private:
    pugi::xml_node node_;
};

} // namespace xml_stream_parser

#endif // XML_STREAM_PARSER_XML_NODE_HPP
