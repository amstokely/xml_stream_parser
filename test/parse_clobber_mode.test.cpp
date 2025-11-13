#include <ut.hpp>
#include "mock_xml_file_system.hpp"
#include "stream.hpp"
#include "test_utils.hpp"

using namespace boost::ut;
using namespace xml_stream_parser;

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
        default_clobber_stream.load_from_xml(get_xml_stream("default_clobber_stream", streams_node),
                                             PugiXmlAdapter{streams_node});
        never_modify_clobber_stream.load_from_xml(get_xml_stream("never_modify_clobber_stream", streams_node),
                                                  PugiXmlAdapter{streams_node});
        append_clobber_stream.load_from_xml(get_xml_stream("append_clobber_stream", streams_node),
                                            PugiXmlAdapter{streams_node});
        truncate_clobber_stream.load_from_xml(PugiXmlAdapter{get_xml_stream("truncate_clobber_stream", streams_node)},
                                              PugiXmlAdapter{streams_node});
        overwrite_clobber_stream.load_from_xml(get_xml_stream("overwrite_clobber_stream", streams_node),
                                               PugiXmlAdapter{streams_node});
        replace_files_clobber_stream.load_from_xml(get_xml_stream("replace_files_clobber_stream", streams_node),
                                                   PugiXmlAdapter{streams_node});
        unrecognized_clobber_stream.load_from_xml(get_xml_stream("unrecognized_clobber_stream", streams_node),
                                                  PugiXmlAdapter{streams_node});
    }
};

void test_xml_parse_clobber_mode() {
    using namespace boost::ut::bdd;
    "xml clobber_mode parsing"_test = [] {
        given("multiple immutable streams with various clobber_mode attributes") = [] {
            const XmlClobberModeStreamParserFixture fixture;

            when("the streams are parsed from XML") = [&] {
                then("the stream 'default_clobber_stream' should default to mode 0") = [&] {
                    expect(fixture.default_clobber_stream.get_clobber_mode() == 0_i);
                };

                then("the stream 'never_modify_clobber_stream' should have mode 0") = [&] {
                    expect(fixture.never_modify_clobber_stream.get_clobber_mode() == 0_i);
                };

                then("the stream 'append_clobber_stream' should have mode 1") = [&] {
                    expect(fixture.append_clobber_stream.get_clobber_mode() == 1_i);
                };

                then("the stream 'truncate_clobber_stream' should have mode 2") = [&] {
                    expect(fixture.truncate_clobber_stream.get_clobber_mode() == 2_i);
                };

                then("the stream 'overwrite_clobber_stream' should have mode 3") = [&] {
                    expect(fixture.overwrite_clobber_stream.get_clobber_mode() == 3_i);
                };

                then("the stream 'replace_files_clobber_stream' should map to mode 2") = [&] {
                    expect(fixture.replace_files_clobber_stream.get_clobber_mode() == 2_i);
                };

                then("the stream 'unrecognized_clobber_stream' should default to mode 0") = [&] {
                    expect(fixture.unrecognized_clobber_stream.get_clobber_mode() == 0_i);
                };
            };
        };
    };
}

int main() {
    test_xml_parse_clobber_mode();
    return 0; // Boost.UT auto-runs all tests on static initialization
}
