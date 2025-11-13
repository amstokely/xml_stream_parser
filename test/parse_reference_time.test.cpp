#include <ut.hpp>
#include "mock_xml_file_system.hpp"
#include "stream.hpp"
#include "test_utils.hpp"

using namespace boost::ut;
using namespace xml_stream_parser;

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
        const auto reference_time_streams_node = reference_time_doc.child("streams");
        const auto reference_time_stream_xml = get_xml_stream("reference_time", reference_time_streams_node);
        reference_time_stream.load_from_xml(reference_time_stream_xml, PugiXmlAdapter{reference_time_streams_node});
        auto default_reference_time_stream_xml = get_xml_stream("default_reference_time", reference_time_streams_node);
        default_reference_time_stream.load_from_xml(default_reference_time_stream_xml,
                                                    PugiXmlAdapter{reference_time_streams_node});
    }
};

void test_xml_parse_reference_time() {
    using namespace boost::ut::bdd;
    "xml reference time parsing"_test = [] {
        given("two immutable streams with and without explicit reference_time attributes") = [] {
            XmlReferenceTimeStreamParserFixture fixture;

            when("the streams are parsed from XML") = [&] {
                then("the stream 'reference_time' should store its explicit value") = [&] {
                    expect(fixture.reference_time_stream.get_reference_time() == "2024-01-01_00:00:00");
                };

                then("the stream 'default_reference_time' should default to 'initial_time'") = [&] {
                    expect(fixture.default_reference_time_stream.get_reference_time() == "initial_time");
                };
            };
        };
    };
}

int main() {
    test_xml_parse_reference_time();
    return 0;
}
