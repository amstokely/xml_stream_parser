#include <ut.hpp>
#include "mock_xml_file_system.hpp"
#include "stream.hpp"
#include "test_utils.hpp"

using namespace boost::ut;
using namespace xml_stream_parser;

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
        auto record_interval_streams_node = record_interval_doc.child("streams");
        auto record_interval_stream_xml = get_xml_stream("record_interval", record_interval_streams_node);
        record_interval_stream.load_from_xml(record_interval_stream_xml, PugiXmlAdapter{record_interval_streams_node});
        auto default_record_interval_stream_xml = get_xml_stream("default_record_interval",
                                                                 record_interval_streams_node);
        default_record_interval_stream.load_from_xml(default_record_interval_stream_xml,
                                                     PugiXmlAdapter{record_interval_streams_node});
    }
};

void test_xml_parse_record_interval() {
    using namespace boost::ut::bdd;
    "xml record interval parsing"_test = [] {
        given("two immutable streams with and without record_interval attributes") = [] {
            const XmlRecordIntervalStreamParserFixture fixture;

            when("the streams are parsed from XML") = [&] {
                then("the stream 'record_interval' should have a record interval of '100'") = [&] {
                    expect(fixture.record_interval_stream.get_record_interval() == "100");
                };

                then("the stream 'default_record_interval' should default to 'none'") = [&] {
                    expect(fixture.default_record_interval_stream.get_record_interval() == "none");
                };
            };
        };
    };
}

int main() {
    test_xml_parse_record_interval();
    return 0;
}
