#include <ut.hpp>
#include "xml_stream_parser.hpp"
#include "mock_xml_file_system.hpp"
#include "stream.hpp"

using namespace boost::ut;
using namespace xml_stream_parser;

auto get_xml_stream(const std::string &stream_id,
                    const pugi::xml_node streams_node) {
    for (auto stream_xml: streams_node.children("immutable_stream")) {
        if (stream_xml.attribute("name").value() == stream_id) {
            return stream_xml;
        }
    }
    throw std::runtime_error("Stream not found");
}

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

struct XmlClobberModeStreamParserFixture {
    Stream default_clobber_stream;
    Stream never_modify_clobber_stream;
    Stream append_clobber_stream;
    Stream truncate_clobber_stream;
    Stream overwrite_clobber_stream;
    Stream replace_files_clobber_stream;
    Stream unrecognized_clobber_stream;

    XmlClobberModeStreamParserFixture() {
        pugi::xml_document doc;
        doc.load_string(R"(
            <streams>
                <immutable_stream name="default_clobber_stream" clobber_mode=""/>
                <immutable_stream name="never_modify_clobber_stream" clobber_mode="never_modify"/>
                <immutable_stream name="append_clobber_stream" clobber_mode="append"/>
                <immutable_stream name="truncate_clobber_stream" clobber_mode="truncate"/>
                <immutable_stream name="overwrite_clobber_stream" clobber_mode="overwrite"/>
                <immutable_stream name="replace_files_clobber_stream" clobber_mode="replace_files"/>
                <immutable_stream name="unrecognized_clobber_stream" clobber_mode="nonsense_value"/>
            </streams>
        )");
        auto streams_node = doc.child("streams");
        default_clobber_stream.load_from_xml(
            get_xml_stream("default_clobber_stream", streams_node),
            streams_node);
        never_modify_clobber_stream.load_from_xml(
            get_xml_stream("never_modify_clobber_stream", streams_node),
            streams_node);
        append_clobber_stream.load_from_xml(
            get_xml_stream("append_clobber_stream", streams_node),
            streams_node);
        truncate_clobber_stream.load_from_xml(get_xml_stream(
            "truncate_clobber_stream", streams_node),
            streams_node);
        overwrite_clobber_stream.load_from_xml(get_xml_stream(
            "overwrite_clobber_stream", streams_node),
            streams_node);
        replace_files_clobber_stream.load_from_xml(get_xml_stream(
            "replace_files_clobber_stream", streams_node),
            streams_node);
        unrecognized_clobber_stream.load_from_xml(get_xml_stream(
            "unrecognized_clobber_stream", streams_node),
            streams_node);
    }


};

void test_xml_parse_clobber_mode() {
    const XmlClobberModeStreamParserFixture fixture;
    expect(fixture.default_clobber_stream.get_iclobber()== 0_i);
    expect(fixture.never_modify_clobber_stream.get_iclobber()== 0_i);
    expect(fixture.append_clobber_stream.get_iclobber()== 1_i);
    expect(fixture.truncate_clobber_stream.get_iclobber()== 2_i);
    expect(fixture.overwrite_clobber_stream.get_iclobber()== 3_i);
    expect(fixture.replace_files_clobber_stream.get_iclobber()== 2_i);
    expect(fixture.unrecognized_clobber_stream.get_iclobber()== 0_i);
}

struct XmlDirectionStreamParserFixture {
    Stream input_stream;
    Stream output_stream;
    Stream inout_stream;
    Stream none_stream;
    Stream default_stream;
    XmlDirectionStreamParserFixture() {
        pugi::xml_document doc;
        doc.load_string(R"(
            <streams>
                <immutable_stream name="input_stream" type="input"/>
                <immutable_stream name="output_stream" type="output"/>
                <immutable_stream name="inout_stream" type="input output"/>
                <immutable_stream name="none_stream" type="none"/>
                <immutable_stream name="default_stream" type=""/>
            </streams>
        )");
        auto streams_node = doc.child("streams");

        input_stream.load_from_xml(
            get_xml_stream("input_stream", streams_node),
            streams_node);
        output_stream.load_from_xml(
            get_xml_stream("output_stream", streams_node),
            streams_node);
        inout_stream.load_from_xml(
            get_xml_stream("inout_stream", streams_node),
            streams_node);
        none_stream.load_from_xml(
            get_xml_stream("none_stream", streams_node),
            streams_node);
        default_stream.load_from_xml(
            get_xml_stream("default_stream", streams_node),
            streams_node);
    }
};

void test_xml_parse_direction_stream_parser() {
    "xml_direction_stream_parser"_test = [] {
        XmlDirectionStreamParserFixture fixture;

        expect(fixture.input_stream.get_itype() == 1_i);
        expect(fixture.output_stream.get_itype() == 2_i);
        expect(fixture.inout_stream.get_itype() == 3_i);
        expect(fixture.none_stream.get_itype() == 4_i);
        expect(fixture.default_stream.get_itype() == 4_i);
    };
}

void test_handle_stream_output_path() {
    "handle_stream_output_path"_test = [] {
        MockFileSystem fs;

        // Case 1: output → writable → no throw → returns normally
        fs.exists_ret = true;
        fs.writable = true;
        expect(nothrow([&] {
            handle_stream_output_path(fs,
                                      2,
                                      "history",
                                      "/path/to/out/history.nc");
        }));

        // Case 2: input-only → should NOT even check filesystem
        fs.exists_ret = false;
        fs.create_success = false;
        expect(nothrow([&] {
            handle_stream_output_path(fs,
                                      1,
                                      "history",
                                      "/path/to/out/history.nc");
        }));

        // Case 3: output stream but not writable → build_stream_path throws → function returns
        fs.exists_ret = true;
        fs.writable = false;
        expect(throws<std::runtime_error>([&] {
            handle_stream_output_path(fs,
                                      2,
                                      "history",
                                      "/path/to/out/history.nc");
        }));
    };
}


struct XmlParsePrecisionFixture {
    Stream single_precision_stream;
    Stream double_precision_stream;
    Stream default_precision_stream;

    XmlParsePrecisionFixture() {
        auto precision_doc = pugi::xml_document{};
        precision_doc.load_string(R"(
            <streams>
                <immutable_stream name="single_precision" precision="single"/>
                <immutable_stream name="double_precision" precision="double"/>
                <immutable_stream name="default_precision" precision=""/>
            </streams>
        )");
        const auto precision_streams_node = precision_doc.child("streams");
        const auto single_precision_stream_xml =
                get_xml_stream("single_precision",
                               precision_streams_node);
        single_precision_stream.load_from_xml(
            single_precision_stream_xml,
            precision_streams_node);
        const auto double_precision_stream_xml =
                get_xml_stream("double_precision",
                               precision_streams_node);
        double_precision_stream.load_from_xml(
            double_precision_stream_xml,
            precision_streams_node);
        const auto default_precision_stream_xml =
                get_xml_stream("default_precision",
                               precision_streams_node);
        default_precision_stream.load_from_xml(
            default_precision_stream_xml,
            precision_streams_node);
    }
};

void test_xml_parse_precision_bytes() {
    XmlParsePrecisionFixture fixture;
    expect(fixture.single_precision_stream.get_iprec() == 4_i);
    expect(fixture.double_precision_stream.get_iprec() == 8_i);
    expect(fixture.default_precision_stream.get_iprec() == 0_i);
}

struct XmlIoStreamParserFixture {
    Stream io_pnetcdf_cdf5_stream;
    Stream io_pnetcdf_stream;
    Stream io_netcdf4_stream;
    Stream io_netcdf_stream;
    Stream io_default_stream;

    XmlIoStreamParserFixture() {
        auto io_doc = pugi::xml_document{};
        io_doc.load_string(R"(
            <streams>
                <immutable_stream name="pnetcdf_cdf5" io_type="pnetcdf,cdf5"/>
                <immutable_stream name="pnetcdf" io_type="pnetcdf"/>
                <immutable_stream name="netcdf4" io_type="netcdf4"/>
                <immutable_stream name="netcdf" io_type="netcdf"/>
                <immutable_stream name="default" io_type=""/>"
            </streams>
        )");
        auto io_streams_node = io_doc.child("streams");
        auto pnetcdf_cdf5_stream_xml =
                get_xml_stream("pnetcdf_cdf5", io_streams_node);
        io_pnetcdf_cdf5_stream.load_from_xml(
            pnetcdf_cdf5_stream_xml,
            io_streams_node);
        auto pnetcdf_stream_xml =
                get_xml_stream("pnetcdf", io_streams_node);
        io_pnetcdf_stream.load_from_xml(pnetcdf_stream_xml,
                                        io_streams_node);
        auto netcdf4_stream_xml =
                get_xml_stream("netcdf4", io_streams_node);
        io_netcdf4_stream.load_from_xml(netcdf4_stream_xml,
                                        io_streams_node);
        auto netcdf_stream_xml =
                get_xml_stream("netcdf", io_streams_node);
        io_netcdf_stream.load_from_xml(netcdf_stream_xml,
                                       io_streams_node);
        auto default_stream_xml =
                get_xml_stream("default", io_streams_node);
        io_default_stream.load_from_xml(default_stream_xml,
                                        io_streams_node);
    }
};

void test_xml_parse_io_type() {
    XmlIoStreamParserFixture fixture;
    expect(fixture.io_pnetcdf_cdf5_stream.get_i_iotype() == 1_i);
    expect(fixture.io_pnetcdf_stream.get_i_iotype() == 0_i);
    expect(fixture.io_netcdf4_stream.get_i_iotype() == 3_i);
    expect(fixture.io_netcdf_stream.get_i_iotype() == 2_i);
    expect(fixture.io_default_stream.get_i_iotype() == 0_i);
}

struct XmlRecordIntervalStreamParserFixture {
    Stream record_interval_stream;
    Stream default_record_interval_stream;

    XmlRecordIntervalStreamParserFixture() {
        auto record_interval_doc = pugi::xml_document{};
        record_interval_doc.load_string(R"(
            <streams>
                <immutable_stream name="record_interval" record_interval="100"/>
                <immutable_stream name="default_record_interval" record_interval=""/>
            </streams>
        )");
        auto record_interval_streams_node = record_interval_doc.child(
            "streams");
        auto record_interval_stream_xml =
                get_xml_stream("record_interval",
                               record_interval_streams_node);
        record_interval_stream.load_from_xml(
            record_interval_stream_xml,
            record_interval_streams_node);
        auto default_record_interval_stream_xml =
                get_xml_stream("default_record_interval",
                               record_interval_streams_node);
        default_record_interval_stream.load_from_xml(
            default_record_interval_stream_xml,
            record_interval_streams_node);
    }
};

void test_xml_parse_record_interval() {
    XmlRecordIntervalStreamParserFixture fixture;
    expect(fixture.record_interval_stream.get_record_interval() ==
           "100");
    expect(
        fixture.default_record_interval_stream.get_record_interval() == "none");
}

struct XmlReferenceTimeStreamParserFixture {
    Stream reference_time_stream;
    Stream default_reference_time_stream;

    XmlReferenceTimeStreamParserFixture() {
        auto reference_time_doc = pugi::xml_document{};
        reference_time_doc.load_string(R"(
            <streams>
                <immutable_stream name="reference_time" reference_time="2024-01-01_00:00:00"/>
                <immutable_stream name="default_reference_time" reference_time=""/>
            </streams>
        )");
        auto reference_time_streams_node = reference_time_doc.child(
            "streams");
        auto reference_time_stream_xml =
                get_xml_stream("reference_time",
                               reference_time_streams_node);
        reference_time_stream.load_from_xml(
            reference_time_stream_xml,
            reference_time_streams_node);
        auto default_reference_time_stream_xml =
                get_xml_stream("default_reference_time",
                               reference_time_streams_node);
        default_reference_time_stream.load_from_xml(
            default_reference_time_stream_xml,
            reference_time_streams_node);
    }
};
void test_xml_parse_reference_time() {
    XmlReferenceTimeStreamParserFixture fixture;
    expect(fixture.reference_time_stream.get_reference_time() ==
           "2024-01-01_00:00:00");
    expect(
        fixture.default_reference_time_stream.get_reference_time() ==
        "initial_time");
}


int main() {
    test_build_stream_path();
    test_xml_parse_clobber_mode();
    test_xml_parse_direction_stream_parser();
    test_xml_parse_io_type();
    test_xml_parse_record_interval();
    test_handle_stream_output_path();
    test_xml_parse_precision_bytes();
    test_xml_parse_reference_time();
}
