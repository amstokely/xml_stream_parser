add_library(xml_stream_parser SHARED xml_stream_parser.hpp)
set_target_properties(xml_stream_parser PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(xml_stream_parser PRIVATE pugixml)
target_include_directories(xml_stream_parser INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
    )