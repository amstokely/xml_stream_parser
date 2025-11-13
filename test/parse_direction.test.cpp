#include <ut.hpp>
#include "mock_xml_file_system.hpp"
#include "stream.hpp"
#include "test_utils.hpp"

using namespace boost::ut;
using namespace xml_stream_parser;

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

int main() {
    test_xml_parse_direction_stream_parser();
}
