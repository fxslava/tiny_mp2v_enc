project(tiny_mp2v_enc)

file(
    GLOB INC_DIRECTORIES
        ./api
)

file(
    GLOB SOURCES
        ./core/parser.cpp
        ./core/decoder.cpp
        ./core/mb_decoder_c.cpp
        ./core/mp2v_vlc.cpp
        ./core/mp2v_hdr.cpp
        ./core/scan_c.cpp
        ./core/quant_c.cpp
        ./core/idct_c.cpp
        ./core/common/cpu.cpp
)

include_directories(
    ${INC_DIRECTORIES}
)

add_library(
	${PROJECT_NAME}
	${SOURCES}
	)
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER INC_DIRECTORIES)
target_include_directories(${PROJECT_NAME} PRIVATE .)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "libs")
	
add_library(
	${PROJECT_NAME}_static
	SHARED
		${SOURCES}
	)
	
set_target_properties(${PROJECT_NAME}_static PROPERTIES PUBLIC_HEADER INC_DIRECTORIES)
target_include_directories(${PROJECT_NAME}_static PRIVATE .)
set_target_properties(${PROJECT_NAME}_static PROPERTIES FOLDER "libs")