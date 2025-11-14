#pragma once
#ifndef XML_STREAM_PARSER_STREAM_HPP
#define XML_STREAM_PARSER_STREAM_HPP

#include <unordered_map>
#include <string>
#include "parse.hpp"

namespace xml_stream_parser {
    template<typename Map>
    auto get_or(const Map &m, const typename Map::key_type &key,
                const typename Map::mapped_type &default_value) -> typename Map::mapped_type {
        if (auto it = m.find(key); it != m.end()) return it->second;
        return default_value;
    }


    template<XmlNode Node>
    class Stream {
    public:
        Stream() = default;

        void load_from_xml(const Node &stream_xml, const Node &streams_root) {
            const auto fields = parse_fields(stream_xml);
            m_stream_id = get_or(fields, "name", "");
            m_type = parse_direction(get_or(fields, "type", ""));
            m_reference_time = parse_reference_time(get_or(fields, "reference_time", ""));
            m_record_interval = parse_record_interval(get_or(fields, "record_interval", ""));
            m_precision = parse_precision_bytes(get_or(fields, "precision", ""));

            m_filename_interval = parse_filename_interval(get_or(fields, "type", ""),
                                                          get_or(fields, "input_interval", ""),
                                                          get_or(fields, "output_interval", ""),
                                                          get_or(fields, "filename_interval", ""), m_stream_id,
                                                          streams_root);

            m_iotype = parse_io_type(get_or(fields, "io_type", ""));
            m_filename_template = get_or(fields, "filename_template", "");
            m_immutable = (stream_xml.name() == "immutable_stream") ? 1 : 0;
            m_clobber_mode = parse_clobber_mode(get_or(fields, "clobber_mode", ""));
        }

        // -------------------------------------------------------------------------
        // Getters
        // -------------------------------------------------------------------------
        [[nodiscard]] constexpr const std::string &get_stream_id() const noexcept { return m_stream_id; }

        [[nodiscard]] constexpr const std::string &get_filename_template() const noexcept {
            return m_filename_template;
        }

        [[nodiscard]] constexpr const std::string &get_filename_interval() const noexcept {
            return m_filename_interval;
        }

        [[nodiscard]] constexpr const std::string &get_reference_time() const noexcept { return m_reference_time; }
        [[nodiscard]] constexpr const std::string &get_record_interval() const noexcept { return m_record_interval; }

        [[nodiscard]] constexpr int get_type() const noexcept { return m_type; }
        [[nodiscard]] constexpr int get_immutable() const noexcept { return m_immutable; }
        [[nodiscard]] constexpr int get_precision() const noexcept { return m_precision; }
        [[nodiscard]] constexpr int get_clobber_mode() const noexcept { return m_clobber_mode; }
        [[nodiscard]] constexpr int get_iotype() const noexcept { return m_iotype; }

    private:
        std::string m_stream_id;
        std::string m_filename_template;
        std::string m_filename_interval;
        std::string m_reference_time;
        std::string m_record_interval;

        int m_type{0};
        int m_immutable{0};
        int m_precision{0};
        int m_clobber_mode{0};
        int m_iotype{0};
    };
} // namespace xml_stream_parser

#endif // XML_STREAM_PARSER_STREAM_HPP
