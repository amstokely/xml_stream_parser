cmake_minimum_required(VERSION 3.20)
project(xml_stream_parser LANGUAGES CXX)

enable_testing()
set(CMAKE_CXX_STANDARD 23)
find_package(pugixml REQUIRED)

include_directories(external)

add_subdirectory(src)
add_subdirectory(test)
