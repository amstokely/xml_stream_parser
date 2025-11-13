#include <ut.hpp>
#include "mock_xml_file_system.hpp"
#include "stream.hpp"
#include "test_utils.hpp"

using namespace boost::ut;
using namespace xml_stream_parser;

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
                                            2,
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
                                    1,
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
                                                2,
                                                "/path/to/out/history.nc");
                                        }));
                };
            };
        };
    };
}

int main() {
    test_handle_stream_output_path();
}
