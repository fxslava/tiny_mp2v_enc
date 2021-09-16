project(tiny_mp2v_parser_test)

include_directories(../sample_common)
include_directories(../../src)
include_directories(../../src/api)

if (WINDOWS)
include_directories(../../vtune/include)
link_directories(../../vtune/lib64)
endif()

file(
    GLOB SOURCES
        *.cpp
        ../sample_common/sample_args.cpp
)

add_executable(
    ${PROJECT_NAME}
    ${SOURCES}
    )

set(
    DEPS_LIBRARIES
        tiny_mp2v_enc
)

if(WINDOWS)
    list(APPEND DEPS_LIBRARIES libittnotify)
else()
    list(APPEND DEPS_LIBRARIES pthread)
endif()
	
target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
		${DEPS_LIBRARIES}
	)
	
if (WINDOWS)
target_link_options(${PROJECT_NAME} PRIVATE -mwindows)
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "samples")