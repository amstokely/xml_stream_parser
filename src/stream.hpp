#pragma once
#ifndef XML_STREAM_PARSER_STREAM_HPP
#define XML_STREAM_PARSER_STREAM_HPP

#include <unordered_map>
#include <string>
#include "parse.hpp"

namespace xml_stream_parser {

/**
 * @brief Returns the value associated with a key in a map or a default value if the key is missing.
 *
 * This helper avoids `operator[]` (which inserts into the map) and provides a safe,
 * non-mutating lookup. It returns by value, which is ideal for maps containing
 * `std::string` or other reasonably copyable types.
 *
 * @tparam Map   A map type supporting `.find()`, `.key_type`, and `.mapped_type`.
 * @param m      The map to query.
 * @param key    The key to search for.
 * @param default_value The value returned if the key is not present.
 *
 * @return The mapped value if found, otherwise `default_value`.
 */
template<typename Map>
auto get_or(const Map& m,
            const typename Map::key_type& key,
            const typename Map::mapped_type& default_value)
    -> typename Map::mapped_type
{
    if (auto it = m.find(key); it != m.end())
        return it->second;
    return default_value;
}

/**
 * @class Stream
 * @brief Represents a parsed MPAS XML stream element.
 *
 * This class extracts and stores the attributes associated with a single
 * `<stream>` or `<immutable_stream>` XML node. The parsed values include:
 * - Stream name
 * - Filename template / interval
 * - Input/output direction
 * - Reference and record intervals
 * - Precision, clobber mode, I/O type, and mutability
 *
 * Parsing is performed through the `load_from_xml()` method, which accepts both
 * the node representing the stream and the XML document root for resolving
 * interval references of the form `"stream:other:input_interval"`.
 *
 * @tparam Node XML node adapter type satisfying the `XmlNode` concept.
 */
template<XmlNode Node>
class Stream {
public:
    Stream() = default;

    /**
     * @brief Loads all stream metadata from the given XML node.
     *
     * This performs:
     * - Attribute lookup (using `parse_fields`)
     * - Default fallback handling (via `get_or`)
     * - Interval resolution (via `parse_interval` / `parse_filename_interval`)
     * - Conversion of attributes into typed values (`parse_direction`, etc.)
     *
     * @param stream_xml   The XML node containing stream attributes.
     * @param streams_root The XML document root used for cross-stream resolution.
     */
    void load_from_xml(const Node& stream_xml, const Node& streams_root) {
        const auto fields = parse_fields(stream_xml);

        m_stream_id         = get_or(fields, "name", "");
        m_type              = parse_direction(get_or(fields, "type", ""));
        m_reference_time    = parse_reference_time(get_or(fields, "reference_time", ""));
        m_record_interval   = parse_record_interval(get_or(fields, "record_interval", ""));
        m_precision         = parse_precision_bytes(get_or(fields, "precision", ""));

        m_filename_interval = parse_filename_interval(
            get_or(fields, "type", ""),
            get_or(fields, "input_interval", ""),
            get_or(fields, "output_interval", ""),
            get_or(fields, "filename_interval", ""),
            m_stream_id,
            streams_root
        );

        m_iotype            = parse_io_type(get_or(fields, "io_type", ""));
        m_filename_template = get_or(fields, "filename_template", "");
        m_immutable         = (stream_xml.name() == "immutable_stream") ? 1 : 0;
        m_clobber_mode      = parse_clobber_mode(get_or(fields, "clobber_mode", ""));
    }

    // -------------------------------------------------------------------------
    // Getters
    // -------------------------------------------------------------------------

    /** @return The unique stream identifier. */
    [[nodiscard]] constexpr const std::string& get_stream_id() const noexcept {
        return m_stream_id;
    }

    /** @return The filename template for output files. */
    [[nodiscard]] constexpr const std::string& get_filename_template() const noexcept {
        return m_filename_template;
    }

    /** @return The computed filename interval. */
    [[nodiscard]] constexpr const std::string& get_filename_interval() const noexcept {
        return m_filename_interval;
    }

    /** @return The reference time used by the stream. */
    [[nodiscard]] constexpr const std::string& get_reference_time() const noexcept {
        return m_reference_time;
    }

    /** @return The record interval used by the stream. */
    [[nodiscard]] constexpr const std::string& get_record_interval() const noexcept {
        return m_record_interval;
    }

    /** @return Stream direction type: 1=input, 2=output, 3=input+output, 4=none. */
    [[nodiscard]] constexpr int get_type() const noexcept { return m_type; }

    /** @return 1 if immutable, 0 if mutable. */
    [[nodiscard]] constexpr int get_immutable() const noexcept { return m_immutable; }

    /** @return Real precision in bytes (4, 8, or 0 for default). */
    [[nodiscard]] constexpr int get_precision() const noexcept { return m_precision; }

    /** @return Clobber mode (0=no modify, 1=append, 2=truncate, 3=overwrite). */
    [[nodiscard]] constexpr int get_clobber_mode() const noexcept { return m_clobber_mode; }

    /** @return I/O type (0=pnetcdf, 1=pnetcdf+cdf5, 2=netcdf, 3=netcdf4/hdf5). */
    [[nodiscard]] constexpr int get_iotype() const noexcept { return m_iotype; }

private:
    // Core string attributes
    std::string m_stream_id;
    std::string m_filename_template;
    std::string m_filename_interval;
    std::string m_reference_time;
    std::string m_record_interval;

    // Parsed integer attributes
    int m_type{0};
    int m_immutable{0};
    int m_precision{0};
    int m_clobber_mode{0};
    int m_iotype{0};
};

} // namespace xml_stream_parser

#endif // XML_STREAM_PARSER_STREAM_HPP
