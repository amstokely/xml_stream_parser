#ifndef XML_STREAM_PARSER_INTERVAL_RESOLVER_HPP
#define XML_STREAM_PARSER_INTERVAL_RESOLVER_HPP
#pragma once

#include <array>
#include <optional>
#include <string>
#include <string_view>
#include <stdexcept>
#include <ranges>
#include "xml_node.hpp"

namespace xml_stream_parser {

class StreamIntervalError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

static constexpr std::array VALID_ATTRS{"input_interval", "output_interval"};

inline void ensure_valid_attribute(std::string_view target_attr) {
    if (std::ranges::find(VALID_ATTRS, target_attr) == VALID_ATTRS.end())
        throw StreamIntervalError("Invalid referenced attribute");
}

template<XmlNode Node>
std::optional<Node> find_stream(const Node &root,
                                std::string_view name,
                                std::string_view tag) {
    for (auto &child : root.children(tag)) {
        if (child.has_attribute("name") && child.get_attribute("name") == name)
            return child;
    }
    return std::nullopt;
}

template<XmlNode Node>
Node resolve_target_stream(const Node &root, std::string_view name) {
    if (auto s = find_stream(root, name, "immutable_stream")) return *s;
    if (auto s = find_stream(root, name, "stream")) return *s;
    throw StreamIntervalError("Referenced stream not found");
}

inline void ensure_not_recursive(const std::string_view streamID,
                                 const std::string_view interval_type,
                                 const std::string_view target_stream,
                                 const std::string_view target_attr) {
    if (target_stream == streamID && target_attr == interval_type)
        throw StreamIntervalError("Self-referencing interval");
}

inline void ensure_resolved_value_is_final(const std::string &resolved) {
    if (resolved == "input_interval" || resolved == "output_interval" || resolved.starts_with("stream:"))
        throw StreamIntervalError("Recursive / unexpandable reference");
}

template<XmlNode Node>
std::string extract_stream_interval_impl(
    std::string_view interval,
    const std::string_view interval_type,
    const std::string_view streamID,
    const Node &streams_root)
{
    using namespace std::string_view_literals;

    if (!interval.starts_with("stream:"sv))
        return std::string(interval);

    interval.remove_prefix(7);

    const auto pos = interval.find(':');
    if (pos == std::string_view::npos)
        throw StreamIntervalError("Malformed interval reference");

    const auto target_stream = interval.substr(0, pos);
    const auto target_attr   = interval.substr(pos + 1);

    ensure_not_recursive(streamID, interval_type, target_stream, target_attr);
    ensure_valid_attribute(target_attr);

    Node target = resolve_target_stream(streams_root, target_stream);
    if (!target.has_attribute(target_attr))
        throw StreamIntervalError("Missing referenced attribute");

    auto resolved = target.get_attribute(target_attr);
    ensure_resolved_value_is_final(resolved);
    return resolved;
}

} // namespace xml_stream_parser


#endif //XML_STREAM_PARSER_INTERVAL_RESOLVER_HPP