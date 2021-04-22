project(tiny_mp2v_parser_test)

include_directories(../sample_common)
include_directories(../../src)
include_directories(../../api)

file(
    GLOB SOURCES
        *.cpp
        ../sample_common/sample_args.cpp
        ../../src/core/parser.cpp
        ../../src/core/mp2v_vlc.cpp
        ../../src/core/common/cpu.cpp
)

add_executable(
    ${PROJECT_NAME} 
    ${SOURCES}
    )