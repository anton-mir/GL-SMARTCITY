cmake_minimum_required(VERSION 2.6)

project(libjsonparse)

set(CMAKE_CXX_EXTRA_FLAGS "-std=c++11 -Wall -O2 -fdata-sections -ffunction-sections -g")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_EXTRA_FLAGS}")
set(JSON_PARSE_OUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../out")
set(JSON_PARSE_OUT_INC_DIR "${JSON_PARSE_OUT_DIR}/include")

include_directories(include)

set(SOURCES src/ccan_json.c
            src/util.c
    )

set(HEADERS include/ccan_json.h
    include/defs.h)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_library(jsonparse ${SOURCES} ${HEADERS})
add_library(jsonparseShared SHARED ${SOURCES} ${HEADERS})
set_target_properties(jsonparseShared PROPERTIES OUTPUT_NAME jsonparse)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
add_executable(main main.c)
target_link_libraries(main jsonparse ${LINKER_FLAGS}) #add executable file and link with static library 

install(TARGETS jsonparse               #installation rules
        ARCHIVE DESTINATION ${JSON_PARSE_OUT_DIR}
        LIBRARY DESTINATION ${JSON_PARSE_OUT_DIR}
        )

install(FILES ${INCLUDES} DESTINATION ${JSON_PARSE_OUT_INC_DIR})
