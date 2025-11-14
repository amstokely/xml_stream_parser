#pragma once
#ifndef XML_STREAM_PARSER_PARSE_HPP
#define XML_STREAM_PARSER_PARSE_HPP

#include <array>
#include <string>
#include <string_view>
#include <unordered_map>
#include <optional>
#include <filesystem>
#include <stdexcept>
#include <ranges>
#include <format>

#include "filesystem.hpp"
#include "parser_concepts.hpp"

namespace xml_stream_parser {

/**
 * @class StreamIntervalError
 * @brief Exception type for errors encountered during stream interval parsing.
 */
class StreamIntervalError final : public std::runtime_error {
public:
    explicit StreamIntervalError(std::string_view msg)
        : std::runtime_error(std::string(msg)) {}
};

// ============================================================================
// Interval validation utilities
// ============================================================================

/// Valid XML stream interval attributes.
constexpr std::array VALID_ATTRS{
    "input_interval", "output_interval"
};

/**
 * @brief Ensures that an attribute name is valid for a stream interval.
 * @throws StreamIntervalError if the attribute is not recognized.
 */
constexpr void ensure_valid_attribute(std::string_view attr) {
    if (std::ranges::find(VALID_ATTRS, attr) == VALID_ATTRS.end())
        throw StreamIntervalError(std::format(
            "Invalid referenced attribute '{}'", attr));
}

/**
 * @brief Ensures that a stream does not reference itself.
 * @throws StreamIntervalError if the stream references its own interval.
 */
constexpr void ensure_not_recursive(std::string_view stream_id,
                                    std::string_view interval_type,
                                    std::string_view target_stream,
                                    std::string_view target_attr) {
    if (target_stream == stream_id && target_attr == interval_type)
        throw StreamIntervalError("Self-referencing interval detected");
}

/**
 * @brief Ensures that a resolved interval value is final and not recursively expandable.
 * @throws StreamIntervalError if the resolved value is another unresolved interval.
 */
constexpr void ensure_resolved_value_is_final(std::string_view resolved) {
    if (resolved == "input_interval" ||
        resolved == "output_interval" ||
        resolved.starts_with("stream:"))
        throw StreamIntervalError("Recursive or unexpandable interval reference");
}

// ============================================================================
// Stream resolution
// ============================================================================

/**
 * @brief Searches for a stream node by name and tag.
 * @return The matching node, or std::nullopt if not found.
 */
template<XmlNode Node>
std::optional<Node> find_stream(const Node& root,
                                std::string_view name,
                                std::string_view tag) {
    for (auto& child : root.children(tag)) {
        if (child.has_attribute("name") &&
            child.get_attribute("name") == name)
            return child;
    }
    return std::nullopt;
}

/**
 * @brief Resolves a referenced stream by name from the given XML root.
 * @throws StreamIntervalError if no matching stream is found.
 */
template<XmlNode Node>
Node resolve_target_stream(const Node& root, std::string_view name) {
    if (auto s = find_stream(root, name, "immutable_stream")) return *s;
    if (auto s = find_stream(root, name, "stream")) return *s;
    throw StreamIntervalError(std::format("Referenced stream '{}' not found", name));
}

// ============================================================================
// Stream interval extraction
// ============================================================================

/**
 * @brief Extracts and resolves an interval reference of the form "stream:other_stream:attribute".
 *
 * @param interval       The interval reference or literal value.
 * @param interval_type  The attribute type ("input_interval" or "output_interval").
 * @param stream_id      The name of the current stream.
 * @param streams_root   The XML root node containing all stream definitions.
 * @return The resolved interval value.
 * @throws StreamIntervalError on invalid, missing, or recursive references.
 */
template<XmlNode Node>
std::string extract_stream_interval(std::string_view interval,
                                    std::string_view interval_type,
                                    std::string_view stream_id,
                                    const Node& streams_root) {
    if (!interval.starts_with("stream:"))
        return std::string(interval);

    interval.remove_prefix(7); // remove "stream:"

    const auto pos = interval.find(':');
    if (pos == std::string_view::npos)
        throw StreamIntervalError("Malformed interval reference (missing ':')");

    const auto target_stream = interval.substr(0, pos);
    const auto target_attr   = interval.substr(pos + 1);

    ensure_not_recursive(stream_id, interval_type, target_stream, target_attr);
    ensure_valid_attribute(target_attr);

    const auto target = resolve_target_stream(streams_root, target_stream);
    if (!target.has_attribute(target_attr))
        throw StreamIntervalError(std::format(
            "Referenced attribute '{}' missing in stream '{}'",
            target_attr, target_stream));

    auto resolved = target.get_attribute(target_attr);
    ensure_resolved_value_is_final(resolved);
    return resolved;
}

/**
 * @brief Wrapper around extract_stream_interval that safely handles empty intervals.
 */
template<XmlNode Node>
std::string parse_interval(std::string_view interval,
                           std::string_view interval_type,
                           std::string_view stream_id,
                           const Node& streams) {
    return interval.empty()
               ? std::string{}
               : extract_stream_interval(interval, interval_type, stream_id, streams);
}

// ============================================================================
// Field parsing
// ============================================================================

/**
 * @brief Extracts all attributes of an XML stream node into a string map.
 * @return A map from attribute name to value.
 */
template<XmlNode Node>
[[nodiscard]] std::unordered_map<std::string, std::string>
parse_fields(const Node& stream_xml) {
    std::unordered_map<std::string, std::string> fields;
    for (const auto& [key, value] : stream_xml.get_attributes())
        fields[key] = value;
    return fields;
}

// ============================================================================
// Filename interval resolution
// ============================================================================

/**
 * @brief Determines the correct filename interval for a stream based on direction and interval attributes.
 *
 * @details
 * - Prefers explicit filename_interval if provided.
 * - Otherwise derives from input/output intervals according to direction.
 */
template<XmlNode Node>
std::string parse_filename_interval(std::string_view direction,
                                    std::string_view interval_in,
                                    std::string_view interval_out,
                                    std::string_view filename_interval,
                                    std::string_view stream_id,
                                    const Node& streams) {
    constexpr auto is_real_interval = [](std::string_view s) noexcept {
        return !s.empty() &&
               s != "initial_only" &&
               s != "final_only" &&
               s != "none";
    };

    const auto resolved_in  = parse_interval(interval_in,  "input_interval",  stream_id, streams);
    const auto resolved_out = parse_interval(interval_out, "output_interval", stream_id, streams);

    const bool for_input  = direction.contains("input");
    const bool for_output = direction.contains("output");

    std::string result{filename_interval};

    auto pick_interval = [&](auto a, auto b) -> std::string {
        return is_real_interval(a) ? a : (is_real_interval(b) ? b : "");
    };

    if (result.empty()) {
        if (for_input && for_output)
            result = pick_interval(resolved_in, resolved_out);
        else if (for_input)
            result = is_real_interval(resolved_in) ? resolved_in : "";
        else if (for_output)
            result = is_real_interval(resolved_out) ? resolved_out : "";
    } else if (result == "input_interval") {
        result = is_real_interval(resolved_in) ? resolved_in : "";
    } else if (result == "output_interval") {
        result = is_real_interval(resolved_out) ? resolved_out : "";
    }

    return result.empty() ? "none" : result;
}

// ============================================================================
// Attribute parsing utilities
// ============================================================================

/// Parses the clobber mode attribute into an integer code.
constexpr int parse_clobber_mode(std::string_view s) noexcept {
    if (s.contains("never_modify"))   return 0;
    if (s.contains("append"))         return 1;
    if (s.contains("truncate") || s.contains("replace_files")) return 2;
    if (s.contains("overwrite"))      return 3;
    return 0;
}

/// Parses the I/O type string into an integer code.
constexpr int parse_io_type(std::string_view s) noexcept {
    if (s.contains("pnetcdf,cdf5")) return 1;
    if (s.contains("pnetcdf"))      return 0;
    if (s.contains("netcdf4"))      return 3;
    if (s.contains("netcdf"))       return 2;
    return 0;
}

/// Parses the direction ("input", "output", or both) into an integer code.
constexpr int parse_direction(std::string_view dir) noexcept {
    const bool in  = dir.contains("input");
    const bool out = dir.contains("output");
    if (in && out) return 3;
    if (in)        return 1;
    if (out)       return 2;
    return 4;
}

/// Returns the reference time, or "initial_time" if not provided.
constexpr std::string_view parse_reference_time(std::string_view ref) noexcept {
    return ref.empty() ? "initial_time" : ref;
}

/// Returns the record interval, or "none" if not provided.
constexpr std::string_view parse_record_interval(std::string_view rec) noexcept {
    return rec.empty() ? "none" : rec;
}

/// Parses the precision attribute into byte width (4 for single, 8 for double).
constexpr int parse_precision_bytes(std::string_view precision) noexcept {
    if (precision.contains("single")) return 4;
    if (precision.contains("double")) return 8;
    return 0;
}

// ============================================================================
// Filesystem output path handling
// ============================================================================

/**
 * @brief Ensures that the directory for a stream's output file exists and is writable.
 * @throws std::runtime_error if directory creation or access fails.
 */
inline void build_stream_path(IXmlFileSystem& fs,
                              std::string_view filename_template) {
    const auto dir = std::filesystem::path(filename_template).parent_path();
    if (dir.empty()) return;

    const auto dir_str = dir.string();
    if (!fs.exists(dir_str) && !fs.create_directories(dir_str))
        throw std::runtime_error(std::format(
            "Failed to create directory '{}'", dir_str));

    if (!fs.can_write(dir_str))
        throw std::runtime_error(std::format(
            "Directory '{}' is not writable", dir_str));
}

/**
 * @brief Creates or validates the output directory if the stream writes files.
 * @param type Stream direction type (2 = output, 3 = input+output).
 */
inline void handle_stream_output_path(IXmlFileSystem& fs,
                                      int type,
                                      std::string_view filename_template) {
    if (type == 2 || type == 3)
        build_stream_path(fs, filename_template);
}

} // namespace xml_stream_parser

#endif // XML_STREAM_PARSER_PARSE_HPP
