#pragma once

#include <unordered_map>
#include <string>
#include <string_view>
#include <filesystem>
#include <stdexcept>

#include "filesystem.hpp"
#include "xml_node.hpp"
#include "interval_resolver.hpp"

namespace xml_stream_parser {
    // -----------------------------------------------------------------------------
    // Public API: Stream interval access
    // -----------------------------------------------------------------------------

    inline std::string extract_stream_interval(
        const std::string &interval,
        const std::string &interval_type,
        const std::string &streamID,
        const pugi::xml_node &streams_root
    ) {
        return extract_stream_interval_impl(
            interval,
            interval_type,
            streamID,
            PugiXmlAdapter{streams_root}
        );
    }

    inline std::string parse_interval(
        const std::string &interval,
        const std::string &interval_type,
        const std::string &streamID,
        const pugi::xml_node &streams
    ) {
        return interval.empty()
                   ? ""
                   : extract_stream_interval(
                       interval, interval_type, streamID, streams);
    }

    // -----------------------------------------------------------------------------
    // Field parsing
    // -----------------------------------------------------------------------------

    inline void parse_fields(
        const pugi::xml_node &stream_xml,
        std::unordered_map<std::string, std::string> &fields
    ) {
        fields.clear();
        for (auto attr: stream_xml.attributes()) {
            fields.emplace(attr.name(), attr.value());
        }
    }

    // -----------------------------------------------------------------------------
    // Filename interval resolution
    // -----------------------------------------------------------------------------

    inline std::string parse_filename_interval(
        const std::string &direction,
        const std::string &interval_in,
        const std::string &interval_out,
        const std::string &interval_in_resolved,
        const std::string &interval_out_resolved,
        const std::string &filename_interval
    ) {
        using namespace std::string_view_literals;

        auto m = filename_interval;

        constexpr auto is_real_interval = [
                ](std::string_view s) noexcept {
            return !s.empty() &&
                   s != "initial_only"sv &&
                   s != "final_only"sv &&
                   s != "none"sv;
        };

        const bool for_input = direction.contains("input"sv);
        const bool for_output = direction.contains("output"sv);

        if (m.empty()) {
            if (for_input && for_output) {
                m = is_real_interval(interval_in)
                        ? interval_in_resolved
                        : is_real_interval(interval_out)
                              ? interval_out_resolved
                              : "";
            } else if (for_input) {
                m = is_real_interval(interval_in)
                        ? interval_in_resolved
                        : "";
            } else if (for_output) {
                m = is_real_interval(interval_out)
                        ? interval_out_resolved
                        : "";
            }
        } else if (m == "input_interval") {
            m = is_real_interval(interval_in)
                    ? interval_in_resolved
                    : "";
        } else if (m == "output_interval") {
            m = is_real_interval(interval_out)
                    ? interval_out_resolved
                    : "";
        }

        return m.empty() ? "none" : m;
    }

    // -----------------------------------------------------------------------------
    // Clobber mode parsing
    // -----------------------------------------------------------------------------

    inline int parse_clobber_mode(const std::string &c) {
        using namespace std::string_view_literals;
        const auto s = std::string_view(c);

        if (s.contains("never_modify"sv)) return 0;
        if (s.contains("append"sv)) return 1;
        if (s.contains("truncate"sv) || s.contains("replace_files"sv))
            return 2;
        if (s.contains("overwrite"sv)) return 3;
        return 0;
    }

    // -----------------------------------------------------------------------------
    // I/O type parsing
    // -----------------------------------------------------------------------------

    inline int parse_io_type(const std::string &iotype) {
        using namespace std::string_view_literals;
        const auto s = std::string_view(iotype);

        if (s.contains("pnetcdf,cdf5"sv)) return 1;
        if (s.contains("pnetcdf"sv)) return 0;
        if (s.contains("netcdf4"sv)) return 3;
        if (s.contains("netcdf"sv)) return 2;
        return 0;
    }

    // -----------------------------------------------------------------------------
    // Direction parsing
    // -----------------------------------------------------------------------------

    inline int parse_direction(const std::string &direction) {
        const bool in = direction.contains("input");
        const bool out = direction.contains("output");

        if (in && out) return 3;
        if (in) return 1;
        if (out) return 2;
        return 4;
    }

    // -----------------------------------------------------------------------------
    // Reference and record interval parsing
    // -----------------------------------------------------------------------------

    inline std::string parse_reference_time(
        const std::string &reference_time) {
        return reference_time.empty() ? "initial_time" : reference_time;
    }

    inline std::string parse_record_interval(
        const std::string &record_interval) {
        return record_interval.empty() ? "none" : record_interval;
    }

    // -----------------------------------------------------------------------------
    // Precision parsing
    // -----------------------------------------------------------------------------

    inline int parse_precision_bytes(
        const std::string &precision_bytes_str) {
        using namespace std::string_view_literals;
        const auto s = std::string_view(precision_bytes_str);
        return s.contains("single"sv)
                   ? 4
                   : s.contains("double"sv)
                         ? 8
                         : 0;
    }

    // -----------------------------------------------------------------------------
    // Filesystem output path handling
    // -----------------------------------------------------------------------------

    inline void build_stream_path(
        IXmlFileSystem &fs,
        const std::string &filename_template
    ) {
        const std::filesystem::path dir = std::filesystem::path(
            filename_template).parent_path();
        if (dir.empty()) return;

        if (!fs.exists(dir.string()) && !fs.create_directories(
                dir.string())) {
            throw std::runtime_error(
                "Failed to create directory '" + dir.string() + "'"
            );
        }

        if (!fs.can_write(dir.string())) {
            throw std::runtime_error(
                "Directory '" + dir.string() + "' is not writable."
            );
        }
    }

    inline void handle_stream_output_path(
        IXmlFileSystem &fs,
        int itype,
        const std::string & /*streamID, unused but preserved*/,
        const std::string &filename_template
    ) {
        // Output-only (2) or input+output (3)
        if (itype == 2 || itype == 3) {
            build_stream_path(fs, filename_template);
        }
    }
} // namespace xml_stream_parser
