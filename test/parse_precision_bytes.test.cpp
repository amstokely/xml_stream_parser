#include <ut.hpp>
#include "mock_xml_file_system.hpp"
#include "stream.hpp"
#include "test_utils.hpp"

using namespace boost::ut;
using namespace xml_stream_parser;

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
        const auto precision_streams_node = precision_doc.child("streams");
        const auto single_precision_stream_xml = get_xml_stream("single_precision", precision_streams_node);
        single_precision_stream.load_from_xml(single_precision_stream_xml, PugiXmlAdapter{precision_streams_node});
        const auto double_precision_stream_xml = get_xml_stream("double_precision", precision_streams_node);
        double_precision_stream.load_from_xml(double_precision_stream_xml, PugiXmlAdapter{precision_streams_node});
        const auto default_precision_stream_xml = get_xml_stream("default_precision", precision_streams_node);
        default_precision_stream.load_from_xml(default_precision_stream_xml, PugiXmlAdapter{precision_streams_node});
    }
};

void test_xml_parse_precision_bytes() {
    using namespace boost::ut::bdd;
    "xml precision attribute parsing"_test = [] {
        given("immutable streams with single, double, and default precision attributes") = [] {
            XmlParsePrecisionFixture fixture;

            when("the streams are parsed from XML") = [&] {
                then("the 'single_precision' stream should have precision 4 bytes") = [&] {
                    expect(fixture.single_precision_stream.get_precision() == 4_i);
                };

                then("the 'double_precision' stream should have precision 8 bytes") = [&] {
                    expect(fixture.double_precision_stream.get_precision() == 8_i);
                };

                then("the 'default_precision' stream should default to precision 0") = [&] {
                    expect(fixture.default_precision_stream.get_precision() == 0_i);
                };
            };
        };
    };
}

int main() {
    test_xml_parse_precision_bytes();
}
