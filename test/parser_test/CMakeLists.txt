project(tiny_mp2v_parser_test)

include_directories(../sample_common)
include_directories(../../src)
include_directories(../../src/api)

#VTune ITT
include_directories(../../vtune/include)
link_directories(../../vtune/lib64)

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
        libittnotify
	)
	
target_link_options(${PROJECT_NAME} PRIVATE -mwindows)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "samples")