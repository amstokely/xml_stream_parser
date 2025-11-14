#pragma once
#ifndef XML_STREAM_PARSER_XML_PARSER_CONCEPTS_HPP
#define XML_STREAM_PARSER_XML_PARSER_CONCEPTS_HPP

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <concepts>

namespace xml_stream_parser {

/**
 * @defgroup xml_concepts XML Concepts
 * @brief Concepts that define the XML backend interface.
 * @{
 */

/**
 * @concept XmlNode
 * @brief A concept describing the minimal interface required for XML node adapters.
 *
 * Any XML wrapper type used by the stream parser must satisfy this concept.
 * The concept ensures that the parser can interact with XML content in a
 * backend-agnostic way — allowing interchangeable implementations such as PugiXML,
 * TinyXML2, or even a custom in-memory XML representation.
 *
 * A type `T` satisfies `XmlNode` if it supports:
 *
 * - Retrieving a single attribute by name:
 *   `std::string get_attribute(std::string_view)`
 *
 * - Checking existence of an attribute:
 *   `bool has_attribute(std::string_view)`
 *
 * - Retrieving child nodes with a given tag name:
 *   `std::vector<T> children(std::string_view)`
 *
 * - Getting the node's element name:
 *   `std::string name()`
 *
 * - Getting all attributes as a map:
 *   `std::unordered_map<std::string, std::string> get_attributes()`
 *
 * This abstraction is central to the design: the entire parser uses only this
 * interface and is never tied directly to PugiXML or any other library.
 */
template<typename T>
concept XmlNode = requires(const T& node,
                           std::string_view key,
                           std::string_view tag)
{
    /// Must return the attribute value or empty string.
    { node.get_attribute(key) }
        -> std::convertible_to<std::string>;

    /// Must return whether the attribute exists.
    { node.has_attribute(key) }
        -> std::convertible_to<bool>;

    /// Must return a list of child XML nodes.
    { node.children(tag) }
        -> std::same_as<std::vector<T>>;

    /// Must return the node element name.
    { node.name() }
        -> std::convertible_to<std::string>;

    /// Must return all attributes as a key/value map.
    { node.get_attributes() }
        -> std::convertible_to<std::unordered_map<std::string, std::string>>;
};

/** @} */ // end of xml_concepts

} // namespace xml_stream_parser

#endif // XML_STREAM_PARSER_XML_PARSER_CONCEPTS_HPP
