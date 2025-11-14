#pragma once
#ifndef XML_STREAM_PARSER_XML_NODE_HPP
#define XML_STREAM_PARSER_XML_NODE_HPP

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <pugixml.hpp>

namespace xml_stream_parser {

/**
 * @class PugiXmlAdapter
 * @brief A lightweight wrapper around `pugi::xml_node` providing
 *        a backend-agnostic interface for XML parsing.
 *
 * This adapter is used to satisfy the `XmlNode` concept across the entire
 * XML stream parser subsystem. By isolating PugiXML here, the higher-level
 * parsing functions remain decoupled from any specific XML library and can be
 * adapted to alternate implementations (TinyXML2, RapidXML, etc.) simply by
 * providing a matching adapter.
 *
 * Responsibilities:
 *  - Retrieve attributes by name.
 *  - Gather all attributes into a map.
 *  - Retrieve named child nodes.
 *  - Report the node's element name.
 */
class PugiXmlAdapter {
public:
    /**
     * @brief Constructs an adapter around a PugiXML node.
     * @param n The underlying PugiXML node.
     */
    explicit PugiXmlAdapter(pugi::xml_node n) noexcept : node_{n} {}

    /**
     * @brief Retrieves an attribute's value by name.
     *
     * The lookup is case-sensitive and returns an empty string if the
     * attribute does not exist.
     *
     * @param key The attribute name.
     * @return The attribute's value, or an empty string if missing.
     */
    [[nodiscard]] std::string get_attribute(std::string_view key) const noexcept {
        if (const auto attr = node_.attribute(std::string{key}.c_str()))
            return attr.value();
        return {};
    }

    /**
     * @brief Retrieves all attributes of this node.
     *
     * @return A map of attribute names and values.
     *
     * @note This copies all attribute strings and is intended for small nodes.
     *       It should not be used in tight inner loops.
     */
    [[nodiscard]] std::unordered_map<std::string, std::string>
    get_attributes() const {
        std::unordered_map<std::string, std::string> attrs;
        attrs.reserve(static_cast<size_t>(
            std::distance(node_.attributes_begin(), node_.attributes_end())
        ));

        for (const auto& attr : node_.attributes())
            attrs.emplace(attr.name(), attr.value());

        return attrs;
    }

    /**
     * @brief Checks whether this XML node has a specific attribute.
     *
     * @param key The attribute name.
     * @return True if the attribute exists, false otherwise.
     */
    [[nodiscard]] bool has_attribute(std::string_view key) const noexcept {
        return node_.attribute(std::string{key}.c_str());
    }

    /**
     * @brief Returns all child nodes with the given tag name.
     *
     * @param tag The child element name.
     * @return A vector of `PugiXmlAdapter` wrappers for matching children.
     */
    [[nodiscard]] std::vector<PugiXmlAdapter>
    children(std::string_view tag) const {
        std::vector<PugiXmlAdapter> result;
        for (const auto& child : node_.children(std::string{tag}.c_str()))
            result.emplace_back(child);
        return result;
    }

    /**
     * @brief Returns the element name of this XML node.
     *
     * @return The node's name (e.g., `"stream"`, `"immutable_stream"`).
     */
    [[nodiscard]] std::string name() const noexcept {
        return node_.name();
    }

private:
    /// The underlying PugiXML node being adapted.
    pugi::xml_node node_;
};

} // namespace xml_stream_parser

#endif // XML_STREAM_PARSER_XML_NODE_HPP
