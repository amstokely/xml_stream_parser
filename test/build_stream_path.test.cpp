#include <ut.hpp>
#include "mock_xml_file_system.hpp"
#include "stream.hpp"
#include "test_utils.hpp"

using namespace boost::ut;
using namespace xml_stream_parser;
using namespace xml_stream_parser::test;

void test_build_stream_path() {
    using namespace boost::ut::bdd;
    "build_stream_path behavior"_test = [] {
        given("a mock file system") = [] {
            MockFileSystem fs;

            when("the target path already exists and is writable") = [& ] {
                fs.exists_ret = true;
                fs.writable = true;

                then("build_stream_path should succeed without throwing") = [&] {
                    expect(nothrow([&] {
                        build_stream_path(fs, "/data/history/file.nc");
                    }));
                };
            };

            when("the path does not exist but can be created successfully") = [&] {
                fs.exists_ret = false;
                fs.create_success = true;
                fs.writable = true;

                then("build_stream_path should create the path successfully") = [&] {
                    expect(nothrow([&] {
                        build_stream_path(fs, "/data/history/file.nc");
                    }));
                };
            };

            when("the path does not exist and directory creation fails") = [&] {
                fs.exists_ret = false;
                fs.create_success = false;
                fs.writable = true;

                then("build_stream_path should throw a runtime_error") = [&] {
                    expect(throws<std::runtime_error>([&] {
                        build_stream_path(fs, "/data/history/file.nc");
                    }));
                };
            };

            when("the path exists but is not writable") = [&] {
                fs.exists_ret = true;
                fs.create_success = true;
                fs.writable = false;

                then("build_stream_path should throw a runtime_error") = [&] {
                    expect(throws<std::runtime_error>([&] {
                        build_stream_path(fs, "/data/history/file.nc");
                    }));
                };
            };

            when("the path is relative (no directory component)") = [& ] {
                fs.exists_ret = false;
                fs.create_success = false;
                fs.writable = false;

                then("build_stream_path should not attempt directory creation and should succeed") = [&] {
                    expect(nothrow([&] {
                        build_stream_path(fs, "file.nc");
                    }));
                };
            };
        };
    };
}

int main() {
}
