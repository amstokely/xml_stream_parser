#include <ut.hpp>
#include "xml_stream_parser.hpp"
#include "mock_xml_file_system.hpp"
#include "stream.hpp"

using namespace boost::ut;
using namespace xml_stream_parser;

void test_build_stream_path() {
    "build_stream_path"_test = [] {
        MockFileSystem fs;

        // Case 1: directory exists and is writable → success
        fs.exists_ret = true;
        fs.writable = true;
        expect(nothrow([&] {
            build_stream_path(fs, "/data/history/file.nc");
        }));

        // Case 2: directory does not exist but can be created → success
        fs.exists_ret = false;
        fs.create_success = true;
        fs.writable = true;
        expect(nothrow([&] {
            build_stream_path(fs, "/data/history/file.nc");
        }));

        // Case 3: directory creation fails → throws
        fs.exists_ret = false;
        fs.create_success = false;
        fs.writable = true;
        expect(throws<std::runtime_error>([&] {
            build_stream_path(fs, "/data/history/file.nc");
        }));

        // Case 4: directory exists but is not writable → throws
        fs.exists_ret = true;
        fs.create_success = true;
        fs.writable = false;
        expect(throws<std::runtime_error>([&] {
            build_stream_path(fs, "/data/history/file.nc");
        }));

        // Case 5: filename has no directory component → nothing happens
        fs.exists_ret = false; // would normally trigger creation
        fs.create_success = false; // but should not be called
        fs.writable = false;
        expect(nothrow([&] {
            build_stream_path(fs, "file.nc");
        }));
    };
}

void test_compute_clobber_mode() {
    "compute_clobber_mode"_test = [] {
        // Default (empty string behaves like NULL)
        auto empty_clobber = parse_clobber_mode("");
        expect(empty_clobber == 0_i);

        // never_modify
        auto never_modify_clobber = parse_clobber_mode("never_modify");
        expect(never_modify_clobber == 0_i);

        // append
        auto append_clobber = parse_clobber_mode("append");
        expect(append_clobber == 1_i);

        // truncate
        auto truncate_clobber = parse_clobber_mode("truncate");
        expect(truncate_clobber == 2_i);

        // replace_files (alias of truncate)
        auto replace_files_clobber =
                parse_clobber_mode("replace_files");
        expect(replace_files_clobber == 2_i);

        // overwrite
        auto overwrite_clobber = parse_clobber_mode("overwrite");
        expect(overwrite_clobber == 3_i);

        // partial substring matches are allowed (same as original strstr)
        auto partial_substring_clobber = parse_clobber_mode(
            "please overwrite this");
        expect(partial_substring_clobber == 3_i);

        // unrecognized → default to 0
        auto unrecognized_clobber =
                parse_clobber_mode("nonsense value");
        expect(unrecognized_clobber == 0_i);
    };
}

void test_compute_direction_type() {
    "compute_direction_type"_test = [] {
        auto input_direction = parse_direction("input");
        expect(input_direction == 1_i);
        auto output_direction = parse_direction("output");
        expect(output_direction == 2_i);
        auto inout_direction = parse_direction("input output");
        expect(inout_direction == 3_i);
        auto none_direction = parse_direction("none");
        expect(none_direction == 4_i);
        auto empty_direction = parse_direction("");
        expect(empty_direction == 4_i);
    };
}

void test_compute_filename_interval() {
    "compute_filename_interval"_test = [] {
        // Input+Output stream
        auto input_output_stream = parse_filename_interval(
            "input output", // direction
            "stream:A", // interval_in
            "stream:B", // interval_out
            "600", // interval_in_resolved
            "300", // interval_out_resolved
            "" // filename_interval unset
        );
        expect(input_output_stream == "600");
        // input wins first if real


        // Input-only stream
        auto input_stream = parse_filename_interval(
            "input",
            "stream:A",
            "none",
            "120",
            "none",
            ""
        );
        expect(input_stream == "120");

        // Output-only stream
        auto output_stream = parse_filename_interval(
            "output",
            "none",
            "stream:A",
            "none",
            "120",
            ""
        );
        expect(output_stream == "120");

        // Explicit filename_interval = input_interval
        auto explcit_input_interval = parse_filename_interval(
            "input",
            "stream:A",
            "none",
            "500",
            "none",
            "input_interval"
        );
        expect(explcit_input_interval == "500");

        // Explicit filename_interval = output_interval
        auto explicit_output_interval = parse_filename_interval(
            "output",
            "none",
            "stream:A",
            "none",
            "400",
            "output_interval"
        );
        expect(explicit_output_interval == "400");

        // filename_interval explicitly set but input interval is "none"
        auto explicit_input_none = parse_filename_interval(
            "input",
            "none",
            "none",
            "IGNORED",
            "IGNORED",
            "input_interval"
        );
        expect(explicit_input_none == "none");

        // Default when nothing matches
        auto default_none = parse_filename_interval(
            "none",
            "none",
            "none",
            "IGNORED",
            "IGNORED",
            ""
        );
        expect(default_none == "none");
    };
}


// ---- Shared XML fixture ----
pugi::xml_node make_stream_fixture() {
    static pugi::xml_document doc;
    doc.load_string(R"(
        <streams>
            <immutable_stream name="s1" input_interval="3h" output_interval="none"/>
            <stream name="s2" input_interval="stream:s1:input_interval" output_interval="6h"/>
        </streams>
    )");
    return doc.child("streams");
}

// ---- Tests ----
void test_no_reference_returns_original() {
    auto streams = make_stream_fixture();

    auto interval = parse_interval("3h", "input_interval", "s2",
                                   streams);
    expect(interval == "3h");
    interval = parse_interval("6h", "output_interval", "s2", streams);
    expect(interval == "6h");
}

void test_compute_valid_stream_reference() {
    const auto streams = make_stream_fixture();

    const auto interval = parse_interval("stream:s1:input_interval",
                                         "input_interval",
                                         "s2", streams);
    expect(interval == "3h");
}

void test_nonexistent_reference_stream_throws() {
    auto streams = make_stream_fixture();

    "nonexistent_reference_stream_throws"_test = [streams] {
        expect(throws<std::runtime_error>([streams] {
            parse_interval("stream:s3:input_interval",
                           "", "s2", streams);
        }));
    };
}


void test_parse_io_type() {
    "compute_io_type"_test = [] {
        auto empty_io_type = parse_io_type("");
        expect(empty_io_type == 0_i);


        // pnetcdf,cdf5 → 1
        auto pnetcdf_cdf5_io_type = parse_io_type("pnetcdf,cdf5");
        expect(pnetcdf_cdf5_io_type == 1_i);

        // pnetcdf → 0
        auto pnetcdf_io_type = parse_io_type("pnetcdf");
        expect(pnetcdf_io_type == 0_i);

        auto netcdf4_io_type = parse_io_type("netcdf4");
        expect(netcdf4_io_type == 3_i);

        // netcdf → 2
        auto netcdf_io_type = parse_io_type("netcdf");
        expect(netcdf_io_type == 2_i);

        // substrings must match (matches original strstr semantics)
        auto pnetcdf_substring_io_type = parse_io_type(
            "use parallel pnetcdf");
        expect(pnetcdf_substring_io_type == 0_i);

        auto netcdf4_substring_io_type = parse_io_type(
            "enable netcdf4 extended mode");
        expect(netcdf4_substring_io_type == 3_i);

        // Unrecognized → default to 0
        auto unknown_io_type = parse_io_type("unknown_type");
        expect(unknown_io_type == 0_i);
    };
}

void test_parse_record_interval() {
    "compute_record_interval"_test = [] {
        auto empty_record_interval = parse_record_interval("");
        expect(empty_record_interval == "none");
        auto explicit_record_interval = parse_record_interval("100");
        expect(explicit_record_interval == "100");
    };
}

void test_parse_reference_time() {
    "compute_reference_time"_test = [] {
        auto empty_reference_time = parse_reference_time("");
        expect(empty_reference_time == "initial_time");
        auto explicit_reference_time = parse_reference_time(
            "2025-01-01");
        expect(explicit_reference_time == "2025-01-01");
    };
}


void test_extract_stream_interval() {
    "extract_stream_interval"_test = [] {
        pugi::xml_document doc;
        doc.load_string(R"(
            <streams>
                <immutable_stream name="A" input_interval="600" output_interval="300"/>
                <stream name="B" input_interval="stream:A:output_interval" output_interval="none"/>
            </streams>
        )");

        auto streams = doc.child("streams");

        // 1. Literal (no reference)
        expect(extract_stream_interval("200", "input_interval", "B",
                                       streams) == "200");

        // 2. Valid reference
        expect(extract_stream_interval("stream:A:output_interval",
                                       "input_interval", "B",
                                       streams) == "300");

        // 3. Missing referenced stream → throws
        expect(throws([&] {
            extract_stream_interval("stream:Z:input_interval",
                                    "input_interval", "B", streams);
        }));

        // 4. Missing referenced attribute → throws
        expect(throws([&] {
            extract_stream_interval("stream:A:does_not_exist",
                                    "input_interval", "B", streams);
        }));

        // 5. Self reference → throws
        expect(throws([&] {
            extract_stream_interval("stream:B:input_interval",
                                    "input_interval", "B", streams);
        }));

        // 6. Unexpandable result (resolves to an interval name, not a value) → throws
        pugi::xml_document doc2;
        doc2.load_string(R"(
            <streams>
                <stream name="C" input_interval="input_interval" output_interval="200"/>
            </streams>
        )");

        expect(throws([&] {
            extract_stream_interval("stream:C:input_interval",
                                    "input_interval", "C",
                                    doc2.child("streams"));
        }));
    };
}


void test_handle_stream_output_path() {
    "handle_stream_output_path"_test = [] {
        MockFileSystem fs;

        // Case 1: output → writable → no throw → returns normally
        fs.exists_ret = true;
        fs.writable = true;
        expect(nothrow([&] {
            handle_stream_output_path(fs, 2, "history",
                                      "/path/to/out/history.nc");
        }));

        // Case 2: input-only → should NOT even check filesystem
        fs.exists_ret = false;
        fs.create_success = false;
        expect(nothrow([&] {
            handle_stream_output_path(fs, 1, "history",
                                      "/path/to/out/history.nc");
        }));

        // Case 3: output stream but not writable → build_stream_path throws → function returns
        fs.exists_ret = true;
        fs.writable = false;
        expect(throws<std::runtime_error>([&] {
            handle_stream_output_path(fs, 2, "history",
                                      "/path/to/out/history.nc");
        }));
    };
}

void test_parse_precision_bytes() {
    "compute_precision_bytes"_test = [] {
        auto single_precision = parse_precision_bytes("single");
        expect(single_precision == 4_i);
        auto double_precision = parse_precision_bytes(
            "double precision");
        expect(double_precision == 8_i);
        auto unknown_precision = parse_precision_bytes("unknown");
        expect(unknown_precision == 0_i);
        auto empty_precision = parse_precision_bytes("");
        expect(empty_precision == 0);
    };
}

void test_parse_single_attribute() {
    "parse single attribute"_test = [] {
        pugi::xml_document doc;
        doc.load_string(R"(
            <stream name="history"/>
        )");

        std::unordered_map<std::string, std::string> fields;
        parse_fields(doc.child("stream"), fields);

        expect(fields["name"] == "history");
        expect(fields.size() == 1_ul);
    };
}

void test_parse_multiple_attributes() {
    "parse multiple attributes"_test = [] {
        pugi::xml_document doc;
        doc.load_string(R"(
            <stream name="history" type="output" interval="300"/>
        )");

        std::unordered_map<std::string, std::string> fields;
        parse_fields(doc.child("stream"), fields);

        expect(fields["name"] == "history");
        expect(fields["type"] == "output");
        expect(fields["interval"] == "300");
        expect(fields.size() == 3_ul);
    };
}

void test_empty_node_produces_empty_map() {
    "parse empty node"_test = [] {
        pugi::xml_document doc;
        doc.load_string(R"(<stream></stream>)");

        std::unordered_map<std::string, std::string> fields;
        parse_fields(doc.child("stream"), fields);

        expect(fields.empty());
    };
}

void test_existing_keys_are_overwritten() {
    "existing values are overwritten"_test = [] {
        pugi::xml_document doc;
        doc.load_string(R"(
            <stream name="new_value"/>
        )");

        std::unordered_map<std::string, std::string> fields;
        fields["name"] = "old_value"; // simulate pre-existing entry

        parse_fields(doc.child("stream"), fields);

        expect(fields["name"] == "new_value");
        expect(fields.size() == 1_ul);
    };
}

void test_load_stream_from_xml() {
    auto doc = make_stream_fixture();
    Stream stream;
    stream.load_from_xml(doc.child("immutable_stream"), doc);
    expect(stream.get_stream_id() == "s1");
    expect(stream.get_itype() == 4_i);
    expect(stream.get_record_interval() == "none");
}

int main() {
    test_build_stream_path();
    test_compute_clobber_mode();
    test_compute_direction_type();
    test_compute_filename_interval();
    test_no_reference_returns_original();
    test_compute_valid_stream_reference();
    test_nonexistent_reference_stream_throws();
    test_parse_io_type();
    test_parse_record_interval();
    test_parse_reference_time();
    test_extract_stream_interval();
    test_handle_stream_output_path();
    test_parse_precision_bytes();
    test_parse_single_attribute();
    test_parse_multiple_attributes();
    test_empty_node_produces_empty_map();
    test_existing_keys_are_overwritten();
    test_load_stream_from_xml();
}
