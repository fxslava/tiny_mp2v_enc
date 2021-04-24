project(tiny_mp2v_parser_test)

include_directories(../sample_common)
include_directories(../../src)
include_directories(../../src/api)

file(
    GLOB SOURCES
        *.cpp
        ../sample_common/sample_args.cpp
)

add_executable(
    ${PROJECT_NAME}
    ${SOURCES}
    )
	
target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
		tiny_mp2v_enc
	)
	
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "samples")