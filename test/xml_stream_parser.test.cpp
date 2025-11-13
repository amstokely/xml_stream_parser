#include <ut.hpp>
#include "mock_xml_file_system.hpp"
#include "stream.hpp"

using namespace boost::ut;
using namespace xml_stream_parser;

auto get_xml_stream(const std::string &stream_id,
                    const pugi::xml_node streams_node) {
    for (auto stream_xml: streams_node.children("immutable_stream")) {
        if (stream_xml.attribute("name").value() == stream_id) {
            return PugiXmlAdapter{stream_xml};
        }
    }
    throw std::runtime_error("Stream not found");
}

void test_build_stream_path() {
    using namespace boost::ut::bdd;
    "build_stream_path behavior"_test = [] {
        given("a mock file system") = [] {
            MockFileSystem fs;

            when("the target path already exists and is writable") = [&
            ] {
                fs.exists_ret = true;
                fs.writable = true;

                then(
                            "build_stream_path should succeed without throwing")
                        = [&] {
                            expect(nothrow([&] {
                                build_stream_path(
                                    fs,
                                    "/data/history/file.nc");
                            }));
                        };
            };

            when(
                        "the path does not exist but can be created successfully")
                    = [&] {
                        fs.exists_ret = false;
                        fs.create_success = true;
                        fs.writable = true;

                        then(
                                    "build_stream_path should create the path successfully")
                                = [&] {
                                    expect(nothrow([&] {
                                        build_stream_path(
                                            fs,
                                            "/data/history/file.nc");
                                    }));
                                };
                    };

            when("the path does not exist and directory creation fails")
                    = [&] {
                        fs.exists_ret = false;
                        fs.create_success = false;
                        fs.writable = true;

                        then(
                                    "build_stream_path should throw a runtime_error")
                                = [&] {
                                    expect(throws<std::runtime_error>(
                                        [&] {
                                            build_stream_path(
                                                fs,
                                                "/data/history/file.nc");
                                        }));
                                };
                    };

            when("the path exists but is not writable") = [&] {
                fs.exists_ret = true;
                fs.create_success = true;
                fs.writable = false;

                then("build_stream_path should throw a runtime_error") =
                        [&] {
                            expect(throws<std::runtime_error>([&] {
                                build_stream_path(
                                    fs,
                                    "/data/history/file.nc");
                            }));
                        };
            };

            when("the path is relative (no directory component)") = [&
            ] {
                fs.exists_ret = false;
                fs.create_success = false;
                fs.writable = false;

                then(
                            "build_stream_path should not attempt directory creation and should succeed")
                        = [&] {
                            expect(nothrow([&] {
                                build_stream_path(fs, "file.nc");
                            }));
                        };
            };
        };
    };
}

struct XmlClobberModeStreamParserFixture {
    Stream<PugiXmlAdapter> default_clobber_stream;
    Stream<PugiXmlAdapter> never_modify_clobber_stream;
    Stream<PugiXmlAdapter> append_clobber_stream;
    Stream<PugiXmlAdapter> truncate_clobber_stream;
    Stream<PugiXmlAdapter> overwrite_clobber_stream;
    Stream<PugiXmlAdapter> replace_files_clobber_stream;
    Stream<PugiXmlAdapter> unrecognized_clobber_stream;

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
            PugiXmlAdapter{streams_node});
        never_modify_clobber_stream.load_from_xml(
            get_xml_stream("never_modify_clobber_stream", streams_node),
            PugiXmlAdapter{streams_node});
        append_clobber_stream.load_from_xml(
            get_xml_stream("append_clobber_stream", streams_node),
            PugiXmlAdapter{streams_node});
        truncate_clobber_stream.load_from_xml(PugiXmlAdapter{get_xml_stream(
                                                  "truncate_clobber_stream",
                                                  streams_node)},
                                              PugiXmlAdapter{streams_node});
        overwrite_clobber_stream.load_from_xml(get_xml_stream(
                                                   "overwrite_clobber_stream",
                                                   streams_node),
                                               PugiXmlAdapter{streams_node});
        replace_files_clobber_stream.load_from_xml(get_xml_stream(
                "replace_files_clobber_stream",
                streams_node),
            PugiXmlAdapter{streams_node});
        unrecognized_clobber_stream.load_from_xml(get_xml_stream(
                "unrecognized_clobber_stream",
                streams_node),
            PugiXmlAdapter{streams_node});
    }
};

void test_xml_parse_clobber_mode() {
    using namespace boost::ut::bdd;
    "xml clobber_mode parsing"_test = [] {
        given(
                    "multiple immutable streams with various clobber_mode attributes")
                = [] {
                    const XmlClobberModeStreamParserFixture fixture;

                    when("the streams are parsed from XML") = [&] {
                        then(
                                    "the stream 'default_clobber_stream' should default to mode 0")
                                = [&] {
                                    expect(
                                        fixture.default_clobber_stream.
                                        get_clobber_mode() == 0_i);
                                };

                        then(
                                    "the stream 'never_modify_clobber_stream' should have mode 0")
                                = [&] {
                                    expect(
                                        fixture.
                                        never_modify_clobber_stream.
                                        get_clobber_mode() == 0_i);
                                };

                        then(
                                    "the stream 'append_clobber_stream' should have mode 1")
                                = [&] {
                                    expect(
                                        fixture.append_clobber_stream.
                                        get_clobber_mode()
                                        == 1_i);
                                };

                        then(
                                    "the stream 'truncate_clobber_stream' should have mode 2")
                                = [&] {
                                    expect(
                                        fixture.truncate_clobber_stream.
                                        get_clobber_mode() == 2_i);
                                };

                        then(
                                    "the stream 'overwrite_clobber_stream' should have mode 3")
                                = [&] {
                                    expect(
                                        fixture.overwrite_clobber_stream
                                        .
                                        get_clobber_mode() == 3_i);
                                };

                        then(
                                    "the stream 'replace_files_clobber_stream' should map to mode 2")
                                = [&] {
                                    expect(
                                        fixture.
                                        replace_files_clobber_stream.
                                        get_clobber_mode() == 2_i);
                                };

                        then(
                                    "the stream 'unrecognized_clobber_stream' should default to mode 0")
                                = [&] {
                                    expect(
                                        fixture.
                                        unrecognized_clobber_stream.
                                        get_clobber_mode() == 0_i);
                                };
                    };
                };
    };
}

struct XmlDirectionStreamParserFixture {
    Stream<PugiXmlAdapter> input_stream;
    Stream<PugiXmlAdapter> output_stream;
    Stream<PugiXmlAdapter> inout_stream;
    Stream<PugiXmlAdapter> none_stream;
    Stream<PugiXmlAdapter> default_stream;

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
            PugiXmlAdapter{streams_node});
        output_stream.load_from_xml(
            get_xml_stream("output_stream", streams_node),
            PugiXmlAdapter{streams_node});
        inout_stream.load_from_xml(
            get_xml_stream("inout_stream", streams_node),
            PugiXmlAdapter{streams_node});
        none_stream.load_from_xml(
            get_xml_stream("none_stream", streams_node),
            PugiXmlAdapter{streams_node});
        default_stream.load_from_xml(
            get_xml_stream("default_stream", streams_node),
            PugiXmlAdapter{streams_node});
    }
};

void test_xml_parse_direction_stream_parser() {
    "xml_direction_stream_parser"_test = [] {
        XmlDirectionStreamParserFixture fixture;

        expect(fixture.input_stream.get_type() == 1_i);
        expect(fixture.output_stream.get_type() == 2_i);
        expect(fixture.inout_stream.get_type() == 3_i);
        expect(fixture.none_stream.get_type() == 4_i);
        expect(fixture.default_stream.get_type() == 4_i);
    };
}

void test_handle_stream_output_path() {
    using namespace boost::ut::bdd;

    "handle_stream_output_path behavior"_test = [] {
        given("a mock file system and a stream output path") = [] {
            MockFileSystem fs;

            when("the stream type is output and the path is writable") =
                    [&] {
                        fs.exists_ret = true;
                        fs.writable = true;

                        then(
                                    "handle_stream_output_path should complete without throwing")
                                = [&] {
                                    expect(nothrow([&] {
                                        handle_stream_output_path(fs,
                                            /*stream_type=*/
                                            2,
                                            "history",
                                            "/path/to/out/history.nc");
                                    }));
                                };
                    };

            when("the stream type is input-only") = [&] {
                fs.exists_ret = false;
                fs.create_success = false;

                then(
                            "handle_stream_output_path should not attempt filesystem operations and should not throw")
                        = [&] {
                            expect(nothrow([&] {
                                handle_stream_output_path(fs,
                                    /*stream_type=*/
                                    1,
                                    "history",
                                    "/path/to/out/history.nc");
                            }));
                        };
            };

            when(
                        "the stream type is output but the path is not writable")
                    = [&] {
                        fs.exists_ret = true;
                        fs.writable = false;

                        then(
                                    "handle_stream_output_path should throw a runtime_error when attempting to write")
                                = [&] {
                                    expect(throws<std::runtime_error>(
                                        [&] {
                                            handle_stream_output_path(
                                                fs,
                                                /*stream_type=*/
                                                2,
                                                "history",
                                                "/path/to/out/history.nc");
                                        }));
                                };
                    };
        };
    };
}

struct XmlParsePrecisionFixture {
    Stream<PugiXmlAdapter> single_precision_stream;
    Stream<PugiXmlAdapter> double_precision_stream;
    Stream<PugiXmlAdapter> default_precision_stream;

    XmlParsePrecisionFixture() {
        auto precision_doc = pugi::xml_document{};
        precision_doc.load_string(R"(
            <streams>
                <immutable_stream name="single_precision" precision="single"/>
                <immutable_stream name="double_precision" precision="double"/>
                <immutable_stream name="default_precision" precision=""/>
            </streams>
        )");
        const auto precision_streams_node = precision_doc.child(
            "streams");
        const auto single_precision_stream_xml =
                get_xml_stream("single_precision",
                               precision_streams_node);
        single_precision_stream.load_from_xml(
            single_precision_stream_xml,
            PugiXmlAdapter{precision_streams_node});
        const auto double_precision_stream_xml =
                get_xml_stream("double_precision",
                               precision_streams_node);
        double_precision_stream.load_from_xml(
            double_precision_stream_xml,
            PugiXmlAdapter{precision_streams_node});
        const auto default_precision_stream_xml =
                get_xml_stream("default_precision",
                               precision_streams_node);
        default_precision_stream.load_from_xml(
            default_precision_stream_xml,
            PugiXmlAdapter{precision_streams_node});
    }
};

void test_xml_parse_precision_bytes() {
    using namespace boost::ut::bdd;
    "xml precision attribute parsing"_test = [] {
        given(
                    "immutable streams with single, double, and default precision attributes")
                = [] {
                    XmlParsePrecisionFixture fixture;

                    when("the streams are parsed from XML") = [&] {
                        then(
                                    "the 'single_precision' stream should have precision 4 bytes")
                                = [&] {
                                    expect(
                                        fixture.single_precision_stream.
                                        get_precision()
                                        == 4_i);
                                };

                        then(
                                    "the 'double_precision' stream should have precision 8 bytes")
                                = [&] {
                                    expect(
                                        fixture.double_precision_stream.
                                        get_precision()
                                        == 8_i);
                                };

                        then(
                                    "the 'default_precision' stream should default to precision 0")
                                = [&] {
                                    expect(
                                        fixture.default_precision_stream
                                        .get_precision()
                                        == 0_i);
                                };
                    };
                };
    };
}

struct XmlIoStreamParserFixture {
    Stream<PugiXmlAdapter> io_pnetcdf_cdf5_stream;
    Stream<PugiXmlAdapter> io_pnetcdf_stream;
    Stream<PugiXmlAdapter> io_netcdf4_stream;
    Stream<PugiXmlAdapter> io_netcdf_stream;
    Stream<PugiXmlAdapter> io_default_stream;

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
            PugiXmlAdapter{io_streams_node});
        auto pnetcdf_stream_xml =
                get_xml_stream("pnetcdf", io_streams_node);
        io_pnetcdf_stream.load_from_xml(pnetcdf_stream_xml,
                                        PugiXmlAdapter{io_streams_node});
        auto netcdf4_stream_xml =
                get_xml_stream("netcdf4", io_streams_node);
        io_netcdf4_stream.load_from_xml(netcdf4_stream_xml,
                                        PugiXmlAdapter{io_streams_node});
        auto netcdf_stream_xml =
                get_xml_stream("netcdf", io_streams_node);
        io_netcdf_stream.load_from_xml(netcdf_stream_xml,
                                       PugiXmlAdapter{io_streams_node});
        auto default_stream_xml =
                get_xml_stream("default", io_streams_node);
        io_default_stream.load_from_xml(default_stream_xml,
                                        PugiXmlAdapter{io_streams_node});
    }
};

void test_xml_parse_io_type() {
    using namespace boost::ut::bdd;
    "xml io_type parsing"_test = [] {
        given("five immutable streams with various io_type attributes")
                = [] {
                    XmlIoStreamParserFixture fixture;

                    when("the streams are parsed from XML") = [&] {
                        then(
                                    "the stream 'pnetcdf_cdf5' should have io_type value 1")
                                = [&] {
                                    expect(
                                        fixture.io_pnetcdf_cdf5_stream.
                                        get_iotype() == 1_i);
                                };

                        then(
                                    "the stream 'pnetcdf' should have io_type value 0")
                                = [&] {
                                    expect(
                                        fixture.io_pnetcdf_stream.
                                        get_iotype()
                                        == 0_i);
                                };

                        then(
                                    "the stream 'netcdf4' should have io_type value 3")
                                = [&] {
                                    expect(
                                        fixture.io_netcdf4_stream.
                                        get_iotype()
                                        == 3_i);
                                };

                        then(
                                    "the stream 'netcdf' should have io_type value 2")
                                = [&] {
                                    expect(
                                        fixture.io_netcdf_stream.
                                        get_iotype() ==
                                        2_i);
                                };

                        then(
                                    "the stream 'default' should default to io_type value 0")
                                = [&] {
                                    expect(
                                        fixture.io_default_stream.
                                        get_iotype()
                                        == 0_i);
                                };
                    };
                };
    };
}

struct XmlRecordIntervalStreamParserFixture {
    Stream<PugiXmlAdapter> record_interval_stream;
    Stream<PugiXmlAdapter> default_record_interval_stream;

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
            PugiXmlAdapter{record_interval_streams_node});
        auto default_record_interval_stream_xml =
                get_xml_stream("default_record_interval",
                               record_interval_streams_node);
        default_record_interval_stream.load_from_xml(
            default_record_interval_stream_xml,
            PugiXmlAdapter{record_interval_streams_node});
    }
};

void test_xml_parse_record_interval() {
    using namespace boost::ut::bdd;
    "xml record interval parsing"_test = [] {
        given(
                    "two immutable streams with and without record_interval attributes")
                = [] {
                    const XmlRecordIntervalStreamParserFixture fixture;

                    when("the streams are parsed from XML") = [&] {
                        then(
                                    "the stream 'record_interval' should have a record interval of '100'")
                                = [&] {
                                    expect(
                                        fixture.record_interval_stream.
                                        get_record_interval() == "100");
                                };

                        then(
                                    "the stream 'default_record_interval' should default to 'none'")
                                = [&] {
                                    expect(
                                        fixture.
                                        default_record_interval_stream.
                                        get_record_interval() ==
                                        "none");
                                };
                    };
                };
    };
}

struct XmlReferenceTimeStreamParserFixture {
    Stream<PugiXmlAdapter> reference_time_stream;
    Stream<PugiXmlAdapter> default_reference_time_stream;

    XmlReferenceTimeStreamParserFixture() {
        auto reference_time_doc = pugi::xml_document{};
        reference_time_doc.load_string(R"(
            <streams>
                <immutable_stream name="reference_time" reference_time="2024-01-01_00:00:00"/>
                <immutable_stream name="default_reference_time" reference_time=""/>
            </streams>
        )");
        const auto reference_time_streams_node = reference_time_doc.
                child(
                    "streams");
        const auto reference_time_stream_xml =
                get_xml_stream("reference_time",
                               reference_time_streams_node);
        reference_time_stream.load_from_xml(
            reference_time_stream_xml,
            PugiXmlAdapter{reference_time_streams_node});
        auto default_reference_time_stream_xml =
                get_xml_stream("default_reference_time",
                               reference_time_streams_node);
        default_reference_time_stream.load_from_xml(
            default_reference_time_stream_xml,
            PugiXmlAdapter{reference_time_streams_node});
    }
};

void test_xml_parse_reference_time() {
    using namespace boost::ut::bdd;
    "xml reference time parsing"_test = [] {
        given(
                    "two immutable streams with and without explicit reference_time attributes")
                = [] {
                    XmlReferenceTimeStreamParserFixture fixture;

                    when("the streams are parsed from XML") = [&] {
                        then(
                                    "the stream 'reference_time' should store its explicit value")
                                = [&] {
                                    expect(
                                        fixture.reference_time_stream.
                                        get_reference_time() ==
                                        "2024-01-01_00:00:00");
                                };

                        then(
                                    "the stream 'default_reference_time' should default to 'initial_time'")
                                = [&] {
                                    expect(
                                        fixture.
                                        default_reference_time_stream.
                                        get_reference_time() ==
                                        "initial_time");
                                };
                    };
                };
    };
}

struct XmlFilenameIntervalStreamParserFixture {
    Stream<PugiXmlAdapter> s1, s2, s3, s4, s5, s6, s7, s8, s9, s10;


    XmlFilenameIntervalStreamParserFixture() {
        auto filename_interval_doc = pugi::xml_document{};
        filename_interval_doc.load_string(R"(
            <streams>
                <immutable_stream name="s1" input_interval="3h" type="input"/>
                <immutable_stream name="s2" input_interval="stream:s1:input_interval" type="input"/>
                <immutable_stream name="s3" input_interval="stream:s1:input_interval" type="output"/>
                <immutable_stream name="s4" output_interval="6h" type="output"/>
                <immutable_stream name="s5" output_interval="stream:s4:output_interval" type="output"/>
                <immutable_stream name="s6" output_interval="stream:s4:output_interval" type="input"/>
                <immutable_stream name="s7" output_interval="stream:s4:output_interval" type="input:output"/>
                <immutable_stream name="s8" input_interval="stream:s1:input_interval" type="input:output"/>
                <immutable_stream name="s9" output_interval="stream:s1:input_interval" type="input:output"/>
                <immutable_stream name="s10" input_interval="stream:s1:input_interval"/>
            </streams>
        )");
        const auto filename_interval_streams_node =
                filename_interval_doc.child(
                    "streams");
        const auto s1_xml =
                get_xml_stream("s1",
                               filename_interval_streams_node);
        s1.load_from_xml(
            s1_xml, PugiXmlAdapter{filename_interval_streams_node});
        auto s2_xml =
                get_xml_stream("s2",
                               filename_interval_streams_node);
        s2.load_from_xml(
            s2_xml,
            PugiXmlAdapter{filename_interval_streams_node});
        auto s3_xml = get_xml_stream("s3",
                                     filename_interval_streams_node);
        s3.load_from_xml(s3_xml, PugiXmlAdapter{filename_interval_streams_node
    });
        auto s4_xml = get_xml_stream("s4",
                                     filename_interval_streams_node);
        s4.load_from_xml(s4_xml, PugiXmlAdapter{filename_interval_streams_node
    });
        auto s5_xml = get_xml_stream("s5",
                                     filename_interval_streams_node);
        s5.load_from_xml(s5_xml, PugiXmlAdapter{filename_interval_streams_node
    });
        auto s6_xml = get_xml_stream("s6",
                                     filename_interval_streams_node);
        s6.load_from_xml(s6_xml, PugiXmlAdapter{filename_interval_streams_node
    });
        auto s7_xml = get_xml_stream("s7",
                                     filename_interval_streams_node);
        s7.load_from_xml(s7_xml, PugiXmlAdapter{filename_interval_streams_node
    });
        auto s8_xml = get_xml_stream("s8",
                                     filename_interval_streams_node);
        s8.load_from_xml(s8_xml, PugiXmlAdapter{filename_interval_streams_node
    });
        auto s9_xml = get_xml_stream("s9",
                                     filename_interval_streams_node);
        s9.load_from_xml(s9_xml, PugiXmlAdapter{filename_interval_streams_node});
        auto s10_xml = get_xml_stream("s10",
                                      filename_interval_streams_node);
        s10.load_from_xml(s10_xml, PugiXmlAdapter{filename_interval_streams_node});
    }
};

void test_xml_parse_input_filename_interval() {
    using namespace boost::ut::bdd;
    "xml filename interval parsing for input streams"_test = [] {
        given("two immutable streams s1 and s2 defined in XML") = [] {
            XmlFilenameIntervalStreamParserFixture fixture;

            when("s1 and s2 are parsed from XML") = [&] {
                then("s1 should have a filename interval of '3h'") = [&
                ] {
                    expect(fixture.s1.get_filename_interval() == "3h");
                };

                then(
                            "s2 should resolve its interval from s1 and also equal '3h'")
                        = [&] {
                            expect(
                                fixture.s2.get_filename_interval() ==
                                "3h");
                        };
            };
        };
    };
}

void test_xml_parse_output_filename_interval() {
    using namespace boost::ut::bdd;
    "xml filename interval parsing for output streams"_test = [] {
        given("two immutable streams s4 and s5 defined in XML") = [] {
            XmlFilenameIntervalStreamParserFixture fixture;

            when("s4 and s5 are parsed from XML") = [&] {
                then("s4 should have a filename interval of '6h'") = [&
                ] {
                    expect(fixture.s4.get_filename_interval() == "6h");
                };

                then(
                            "s5 should resolve its interval from s4 and also equal '6h'")
                        = [&] {
                            expect(
                                fixture.s5.get_filename_interval() ==
                                "6h");
                };
            };
        };
    };
}

void test_xml_parse_different_direction_filename_interval() {
    using namespace boost::ut::bdd;
    "xml filename interval parsing with different stream directions"_test = [] {
        given("four immutable streams s1, s3, s4, and s6 defined in XML") = [] {
            XmlFilenameIntervalStreamParserFixture fixture;

            when("s1, s3, s4, and s6 are parsed from XML") = [&] {
                then("s1 should have a filename interval of '3h'") = [&
                ] {
                    expect(fixture.s1.get_filename_interval() == "3h");
                };

                then(
                            "s3 should have a filename interval of 'none' since it is output only and references an input interval")
                        = [&] {
                            expect(
                                fixture.s3.get_filename_interval() ==
                                "none");
                };
                then ("s4 should have a filename interval of '6h'") = [&
                ] {
                    expect(fixture.s4.get_filename_interval() == "6h");
                };

                then(
                            "s6 should have a filename interval of " "'none' since it is input only and references an output interval")
                        = [&] {
                            expect(
                                fixture.s6.get_filename_interval() ==
                                "none");
                };
            };
        };
    };
}

void test_xml_parse_input_output_filename_interval() {
    using namespace boost::ut::bdd;
    "xml input output filename interval parsing"_test = [] {
        given("three immutable streams s7, s8, and s9 defined in XML") = [] {
            XmlFilenameIntervalStreamParserFixture fixture;

            when("s7, s8, and s9 are parsed from XML") = [&] {
                then("s7 should have a filename interval of '6h'") = [&
                ] {
                    expect(fixture.s7.get_filename_interval() == "6h");
                };
                then("s8 should have a filename interval of '3h'") = [&
                ] {
                    expect(fixture.s8.get_filename_interval() == "3h");
                };
                then("s9 should have a filename interval of '3h'") = [&
                ] {
                    expect(fixture.s9.get_filename_interval() == "3h");
                };
            };
        };
    };
}

void test_xml_parse_typeless_filename_interval() {
    using namespace boost::ut::bdd;
    "xml typeless filename interval parsing"_test = [] {
        given("one immutable stream s10 defined in XML") = [] {
            XmlFilenameIntervalStreamParserFixture fixture;

            when("s10 is parsed from XML") = [&] {
                then("s10 should have a filename interval of 'none'") = [&
                ] {
                    expect(fixture.s10.get_filename_interval() == "none");
                };
            };
        };
    };
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
    test_xml_parse_input_filename_interval();
    test_xml_parse_output_filename_interval();
    test_xml_parse_different_direction_filename_interval();
    test_xml_parse_input_output_filename_interval();
    test_xml_parse_typeless_filename_interval();
}
