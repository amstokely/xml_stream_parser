#ifndef XML_STREAM_PARSER_STREAM_HPP
#define XML_STREAM_PARSER_STREAM_HPP
#include <unordered_map>
#include "parse.hpp"

namespace xml_stream_parser {
    class Stream {
        std::string m_stream_id;
        int m_itype{};
        std::string m_filename_template;
        std::string m_filename_interval;
        std::string m_reference_time;
        std::string m_record_interval;
        int m_immutable{};
        int m_iprec{};
        int m_iclobber{};
        int m_i_iotype{};

    public:
        Stream() = default;

        void load_from_xml(const pugi::xml_node &stream_xml,
                           const pugi::xml_node &streams_root) {
            std::unordered_map<std::string, std::string> fields;
            parse_fields(stream_xml, fields);
            m_stream_id = fields.at("name");
            m_itype = parse_direction(fields["type"]);
            m_reference_time = parse_reference_time(
                fields["reference_time"]);
            m_record_interval = parse_record_interval(
                fields["record_interval"]);
            m_iprec = parse_precision_bytes(fields["precision"]);
            auto interval_in_resolved = parse_interval(
                fields["input_interval"],
                "input_interval", m_stream_id,
                streams_root);
            auto interval_out_resolved = parse_interval(
                fields["output_interval"],
                "output_interval", m_stream_id,
                streams_root);
            m_filename_interval = parse_filename_interval(
                fields["type"], fields["input_interval"],
                fields["output_interval"],
                interval_in_resolved, interval_out_resolved,
                fields["filename_interval"]
                );

            m_i_iotype = parse_io_type(fields["io_type"]);
            m_filename_template = fields["filename_template"];
            m_immutable = stream_xml.name() == std::string("immutable_stream") ? 1 : 0;
            m_iclobber = parse_clobber_mode(fields["clobber_mode"]);
        }

        [[nodiscard]] const std::string &get_stream_id() const {
            return m_stream_id;
        }
        [[nodiscard]] const std::string &get_filename_template() const {
            return m_filename_template;
        }
        [[nodiscard]] const std::string &get_filename_interval() const {
            return m_filename_interval;
        }
        [[nodiscard]] const std::string &get_reference_time() const {
            return m_reference_time;
        }
        [[nodiscard]] const std::string &get_record_interval() const {
            return m_record_interval;
        }
        [[nodiscard]] int get_itype() const {
            return m_itype;
        }
        [[nodiscard]] int get_immutable() const {
            return m_immutable;
        }
        [[nodiscard]] int get_iprec() const {
            return m_iprec;
        }
        [[nodiscard]] int get_iclobber() const {
            return m_iclobber;
        }
        [[nodiscard]] int get_i_iotype() const {
            return m_i_iotype;
        }
    };
}

#endif //XML_STREAM_PARSER_STREAM_HPP