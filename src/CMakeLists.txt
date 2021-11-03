project(tiny_mp2v_enc)

set(CMAKE_CXX_STANDARD 11)

file(
    GLOB SOURCES
        ./core/encoder.cpp
)

if(WIN32)
    set(
        CMAKE_CXX_FLAGS
            "${CMAKE_CXX_FLAGS} /fp:precise"
    )
elseif(UNIX)
    set(
        CMAKE_CXX_FLAGS
            "${CMAKE_CXX_FLAGS} -Ofast -g0 -Wall"
    )
endif()

add_library(
	${PROJECT_NAME}
	${SOURCES}
	)
	
target_include_directories(${PROJECT_NAME} PRIVATE .)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")