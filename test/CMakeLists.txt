project(tiny_mp2v_parser_test)

file(
    GLOB_RECURSE SOURCES
        *.cpp
)

include_directories(../src)

add_executable(tiny_mp2v_parser_test parser_test.cpp)