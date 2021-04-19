project(tiny_mp2v_parser_test)

include_directories(../sample_common)
include_directories(../../src)

file(
    GLOB SOURCES
        *.cpp
        ../sample_common/sample_args.cpp
        ../../src/core/parser.cpp
        ../../src/core/mp2v_vlc_tbl.cpp
)

add_executable(
    ${PROJECT_NAME} 
    ${SOURCES}
    )