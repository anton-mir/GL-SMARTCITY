project(SBC)
cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if("${BUILD_SHARED_LIBS}" STREQUAL "")
    message(STATUS "Build project with shared libraries. To build project with static libraries add definition -DBUILD_SHARED_LIBS=OFF ")
    SET(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared library" FORCE)
endif()

set(LIB_JSON_PARSER_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/libjsonparse CACHE PATH "Path to the libjsonparse project sources" FORCE)
add_subdirectory(${LIB_JSON_PARSER_ROOT_DIR} ${CMAKE_CURRENT_BINARY_DIR}/libjsonparse)

set(LIB_GLOG_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/glog CACHE PATH "Path to the glog project sources" FORCE)
add_subdirectory(${LIB_GLOG_ROOT_DIR} glog)

set(LIB_GTEST_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/gtest CACHE PATH "Path to the gest project sources" FORCE)
add_subdirectory(${LIB_GTEST_ROOT_DIR} gtest)

set(LIB_RESTCLIENT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/restclient-cpp CACHE PATH "Path to the restclient project sources" FORCE)
add_subdirectory(${LIB_RESTCLIENT_ROOT_DIR} restclient)

add_subdirectory(src)
