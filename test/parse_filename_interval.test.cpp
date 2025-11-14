#include <ut.hpp>
#include "stream.hpp"
#include "test_utils.hpp"

using namespace boost::ut;
using namespace boost::ut::bdd;
using namespace xml_stream_parser;


struct XmlFilenameIntervalStreamParserFixture {
    Stream<PugiXmlAdapter> s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11,
            s12, s13, s14, s15, s16, s17, s18;

    XmlFilenameIntervalStreamParserFixture() {
        pugi::xml_document doc;
        doc.load_string(R"(
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
                <immutable_stream name="s11" filename_interval="input_interval" input_interval="initial_only" type="input"/>
                <immutable_stream name="s12" filename_interval="input_interval" input_interval="final_only" type="input"/>
                <immutable_stream name="s13" filename_interval="output_interval" output_interval="initial_only" type="output"/>
                <immutable_stream name="s14" filename_interval="output_interval" output_interval="final_only" type="output"/>
                <immutable_stream name="s15" filename_interval="output_interval" output_interval="4h" type="output"/>
                <immutable_stream name="s16" filename_interval="input_interval" input_interval="4h" type="input"/>
                <immutable_stream name="s17" filename_interval="output_interval" output_interval="stream:s4:output_interval" type="output"/>
                <immutable_stream name="s18" filename_interval="input_interval" input_interval="stream:s1:input_interval" type="input"/>
            </streams>
        )");

        const auto streams = PugiXmlAdapter{doc.child("streams")};

        auto load = [&](auto &s, const std::string &name) {
            s.load_from_xml(get_xml_stream(name, doc.child("streams")),
                            streams);
        };

        load(s1, "s1");
        load(s2, "s2");
        load(s3, "s3");
        load(s4, "s4");
        load(s5, "s5");
        load(s6, "s6");
        load(s7, "s7");
        load(s8, "s8");
        load(s9, "s9");
        load(s10, "s10");
        load(s11, "s11");
        load(s12, "s12");
        load(s13, "s13");
        load(s14, "s14");
        load(s15, "s15");
        load(s16, "s16");
        load(s17, "s17");
        load(s18, "s18");
    }
};

// -----------------------------------------------------------------------------
// Tests
// -----------------------------------------------------------------------------

int main() {
    "input stream filename intervals"_test = [] {
        given("immutable streams s1 and s2 defined in XML") = [] {
            XmlFilenameIntervalStreamParserFixture fx;
            when("parsed from XML") = [&] {
                then("s1 should have filename_interval = '3h'") = [&] {
                    expect(eq(fx.s1.get_filename_interval(), "3h"_s));
                };
                then(
                            "s2 should resolve filename_interval from s1, also '3h'")
                        = [&] {
                            expect(eq(fx.s2.get_filename_interval(),
                                      "3h"_s));
                        };
            };
        };
    };

    // -------------------------------------------------------------------------
    // Output streams: s4 (explicit), s5 (derived from s4)
    // -------------------------------------------------------------------------
    "output stream filename intervals"_test = [] {
        given("immutable streams s4 and s5 defined in XML") = [] {
            XmlFilenameIntervalStreamParserFixture fx;
            when("parsed from XML") = [&] {
                then("s4 should have filename_interval = '6h'") = [&] {
                    expect(eq(fx.s4.get_filename_interval(), "6h"_s));
                };
                then(
                            "s5 should resolve filename_interval from s4, also '6h'")
                        = [&] {
                            expect(eq(fx.s5.get_filename_interval(),
                                      "6h"_s));
                        };
            };
        };
    };

    // -------------------------------------------------------------------------
    // Mixed direction: s3 (output→input mismatch), s6 (input→output mismatch)
    // -------------------------------------------------------------------------
    "mixed direction filename intervals"_test = [] {
        given("immutable streams s1, s3, s4, and s6 defined in XML") = [
        ] {
            const XmlFilenameIntervalStreamParserFixture fx;
            when("parsed from XML") = [&] {
                then("s1 should have filename_interval = '3h'") = [&] {
                    expect(eq(fx.s1.get_filename_interval(), "3h"_s));
                };
                then(
                            "s3 should have filename_interval = 'none' (output referencing input)")
                        = [&] {
                            expect(eq(fx.s3.get_filename_interval(),
                                      "none"_s));
                        };
                then("s4 should have filename_interval = '6h'") = [&] {
                    expect(eq(fx.s4.get_filename_interval(), "6h"_s));
                };
                then(
                            "s6 should have filename_interval = 'none' (input referencing output)")
                        = [&] {
                            expect(eq(fx.s6.get_filename_interval(),
                                      "none"_s));
                        };
            };
        };
    };

    // -------------------------------------------------------------------------
    // Input:Output streams: both intervals valid and resolvable
    // -------------------------------------------------------------------------
    "input:output filename intervals"_test = [] {
        given("immutable streams s7, s8, and s9 defined in XML") = [] {
            XmlFilenameIntervalStreamParserFixture fx;
            when("parsed from XML") = [&] {
                then("s7 should have filename_interval = '6h'") = [&] {
                    expect(eq(fx.s7.get_filename_interval(), "6h"_s));
                };
                then("s8 should have filename_interval = '3h'") = [&] {
                    expect(eq(fx.s8.get_filename_interval(), "3h"_s));
                };
                then("s9 should have filename_interval = '3h'") = [&] {
                    expect(eq(fx.s9.get_filename_interval(), "3h"_s));
                };
            };
        };
    };

    // -------------------------------------------------------------------------
    // Typeless stream (no input/output type)
    // -------------------------------------------------------------------------
    "typeless stream filename intervals"_test = [] {
        given("immutable stream s10 defined in XML") = [] {
            XmlFilenameIntervalStreamParserFixture fx;
            when("parsed from XML") = [&] {
                then("s10 should have filename_interval = 'none'") = [&
                ] {
                    expect(eq(fx.s10.get_filename_interval(),
                              "none"_s));
                };
            };
        };
    };
    "filename interval explicit input_interval initial_only"_test = [] {
        given("immutable stream s11 defined in XML") = [] {
            XmlFilenameIntervalStreamParserFixture fx;
            when("parsed from XML") = [&] {
                then("s11 should have filename_interval = 'none'") = [&
                ] {
                    expect(eq(fx.s11.get_filename_interval(),
                              "none"_s));
                };
            };
        };
    };
    "filename interval explicit input_interval final_only"_test = [] {
        given("immutable stream s12 defined in XML") = [] {
            XmlFilenameIntervalStreamParserFixture fx;
            when("parsed from XML") = [&] {
                then("s12 should have filename_interval = 'none'") = [&
                ] {
                    expect(eq(fx.s12.get_filename_interval(),
                              "none"_s));
                };
            };
        };
    };
    "filename interval explicit output_interval initial_only"_test = [] {
        given("immutable stream s13 defined in XML") = [] {
            XmlFilenameIntervalStreamParserFixture fx;
            when("parsed from XML") = [&] {
                then("s13 should have filename_interval = 'none'") = [&
                ] {
                    expect(eq(fx.s13.get_filename_interval(),
                              "none"_s));
                };
            };
        };
    };
    "filename interval explicit output_interval final_only"_test = [] {
        given("immutable stream s14 defined in XML") = [] {
            XmlFilenameIntervalStreamParserFixture fx;
            when("parsed from XML") = [&] {
                then("s14 should have filename_interval = 'none'") = [&
                ] {
                    expect(eq(fx.s14.get_filename_interval(),
                              "none"_s));
                };
            };
        };
    };
    "filename interval explicit output_interval time interval"_test = [] {
        given("immutable stream s15 defined in XML") = [] {
            XmlFilenameIntervalStreamParserFixture fx;
            when("parsed from XML") = [&] {
                then("s15 should have filename_interval = '4h'") = [&
                ] {
                    expect(eq(fx.s15.get_filename_interval(),
                              "4h"_s));
                };
            };
        };
    };
    "filename interval explicit input_interval time interval"_test = [] {
        given("immutable stream s16 defined in XML") = [] {
            XmlFilenameIntervalStreamParserFixture fx;
            when("parsed from XML") = [&] {
                then("s16 should have filename_interval = '4h'") = [&
                ] {
                    expect(eq(fx.s16.get_filename_interval(),
                              "4h"_s));
                };
            };
        };
    };
    "filename interval explicit output_interval ref"_test = [] {
        given("immutable stream s17 defined in XML") = [] {
            XmlFilenameIntervalStreamParserFixture fx;
            when("parsed from XML") = [&] {
                then("s17 should have filename_interval = '6h'") = [&
                ] {
                    expect(eq(fx.s17.get_filename_interval(),
                              "6h"_s));
                };
            };
        };
    };
    "filename interval explicit input_interval ref"_test = [] {
        given("immutable stream s18 defined in XML") = [] {
            XmlFilenameIntervalStreamParserFixture fx;
            when("parsed from XML") = [&] {
                then("s18 should have filename_interval = '3h'") = [&
                ] {
                    expect(eq(fx.s18.get_filename_interval(),
                              "3h"_s));
                };
            };
        };
    };
    return 0;
};
