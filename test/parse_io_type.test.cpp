#include <ut.hpp>
#include "mock_xml_file_system.hpp"
#include "stream.hpp"
#include "test_utils.hpp"

using namespace boost::ut;
using namespace xml_stream_parser;

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
        auto pnetcdf_cdf5_stream_xml = get_xml_stream("pnetcdf_cdf5", io_streams_node);
        io_pnetcdf_cdf5_stream.load_from_xml(pnetcdf_cdf5_stream_xml, PugiXmlAdapter{io_streams_node});
        auto pnetcdf_stream_xml = get_xml_stream("pnetcdf", io_streams_node);
        io_pnetcdf_stream.load_from_xml(pnetcdf_stream_xml, PugiXmlAdapter{io_streams_node});
        auto netcdf4_stream_xml = get_xml_stream("netcdf4", io_streams_node);
        io_netcdf4_stream.load_from_xml(netcdf4_stream_xml, PugiXmlAdapter{io_streams_node});
        auto netcdf_stream_xml = get_xml_stream("netcdf", io_streams_node);
        io_netcdf_stream.load_from_xml(netcdf_stream_xml, PugiXmlAdapter{io_streams_node});
        auto default_stream_xml = get_xml_stream("default", io_streams_node);
        io_default_stream.load_from_xml(default_stream_xml, PugiXmlAdapter{io_streams_node});
    }
};

void test_xml_parse_io_type() {
    using namespace boost::ut::bdd;
    "xml io_type parsing"_test = [] {
        given("five immutable streams with various io_type attributes") = [] {
            XmlIoStreamParserFixture fixture;

            when("the streams are parsed from XML") = [&] {
                then("the stream 'pnetcdf_cdf5' should have io_type value 1") = [&] {
                    expect(fixture.io_pnetcdf_cdf5_stream.get_iotype() == 1_i);
                };

                then("the stream 'pnetcdf' should have io_type value 0") = [&] {
                    expect(fixture.io_pnetcdf_stream.get_iotype() == 0_i);
                };

                then("the stream 'netcdf4' should have io_type value 3") = [&] {
                    expect(fixture.io_netcdf4_stream.get_iotype() == 3_i);
                };

                then("the stream 'netcdf' should have io_type value 2") = [&] {
                    expect(fixture.io_netcdf_stream.get_iotype() == 2_i);
                };

                then("the stream 'default' should default to io_type value 0") = [&] {
                    expect(fixture.io_default_stream.get_iotype() == 0_i);
                };
            };
        };
    };
}

int main() {
    test_xml_parse_io_type();
}
