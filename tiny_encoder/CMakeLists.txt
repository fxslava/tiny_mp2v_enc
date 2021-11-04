project(tiny_mp2v_enc_sample)

include_directories(../sample_common)
include_directories(../src)

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

if(WIN32)
else()
    list(APPEND DEPS_LIBRARIES pthread)
endif()
	
target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
		${DEPS_LIBRARIES}
	)
	
if (WIN32)
target_link_options(${PROJECT_NAME} PRIVATE -mwindows)
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "samples")