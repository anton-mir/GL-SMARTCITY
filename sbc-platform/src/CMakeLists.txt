cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -fdata-sections -ffunction-sections -fPIC -DBOOST_MATH_DISABLE_FLOAT128")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O2")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -ggdb -O0")
set(CMAKE_DEBUG_POSTFIX "" CACHE STRING "Don't add any postfix for debug version" FORCE)

if("${BUILD_SHARED_LIBS}" STREQUAL "")
    message(STATUS "Build project with shared libraries.
     To build project with static libraries add definition -DBUILD_SHARED_LIBS=OFF ")
    SET(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared library" FORCE)
endif()

if("${LIB_JSON_PARSER_ROOT_DIR}" STREQUAL "")
    set(LIB_JSON_PARSER_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../lib/libjsonparse CACHE PATH "Path to the libjsonparse project sources" FORCE)
    add_subdirectory(${LIB_JSON_PARSER_ROOT_DIR} ${CMAKE_CURRENT_BINARY_DIR}/libjsonparse)
endif()

if("${LIB_GLOG_ROOT_DIR}" STREQUAL "")
    set(LIB_GLOG_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../lib/glog CACHE PATH "Path to the glog project sources" FORCE)
    add_subdirectory(${LIB_GLOG_ROOT_DIR} glog)
endif()

if("${LIB_GTEST_ROOT_DIR}" STREQUAL "")
    set(LIB_GTEST_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../lib/gtest CACHE PATH "Path to the gest project sources" FORCE)
    add_subdirectory(${LIB_GTEST_ROOT_DIR} gtest)
endif()

add_subdirectory(CloudHub)
add_subdirectory(IC-Lib)
add_subdirectory(pits/xlgyrod)
add_subdirectory(sbc-car)

