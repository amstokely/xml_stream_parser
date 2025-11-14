#ifndef XML_STREAM_PARSER_XML_PARSER_CONCEPTS_HPP
#define XML_STREAM_PARSER_XML_PARSER_CONCEPTS_HPP


namespace xml_stream_parser {

    template<typename T>
    concept XmlNode = requires(const T &node, std::string_view key, std::string_view tag) {
        { node.get_attribute(key) } -> std::convertible_to<std::string>;
        { node.has_attribute(key) } -> std::convertible_to<bool>;
        { node.children(tag) }      -> std::same_as<std::vector<T>>;
        {node.name() } -> std::convertible_to<std::string>;
        {node.get_attributes()} -> std::convertible_to<std::unordered_map<std::string, std::string>>;
    };
    }

#endif //XML_STREAM_PARSER_XML_PARSER_CONCEPTS_HPP
