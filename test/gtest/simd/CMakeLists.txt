project(simd_test)

include_directories(../common)
include_directories(../../../src)
include_directories(../../../src/api)

file(
    GLOB SOURCES
        *.cpp
)

add_executable(
    ${PROJECT_NAME}
    ${SOURCES}
    )

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
        tiny_mp2v_enc
        gtest_main
  )

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "tests")

# automatic discovery of unit tests
include(GoogleTest)
gtest_discover_tests(
    ${PROJECT_NAME}
    PROPERTIES
        LABELS "unit"
    DISCOVERY_TIMEOUT  # how long to wait (in seconds) before crashing
        240
  )