cmake_minimum_required(VERSION 3.0)

project(cluster-app)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif(NOT CMAKE_BUILD_TYPE)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
execute_process(COMMAND rm -rf ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

if("${BUILD_SHARED_LIBS}" STREQUAL "")
    message(STATUS "Build project with shared libraries. To build project with static libraries add definition -DBUILD_SHARED_LIBS=OFF ")
    SET(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared library" FORCE)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -fdata-sections -ffunction-sections -fPIC")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O2 -DDISABLE_DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -ggdb -O0")
set(CMAKE_DEBUG_POSTFIX "" CACHE STRING "Don't add any postfix for debug version" FORCE)

#set(LIB_V2X_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../V2X")

#if(NOT ASN1_ROOT_DIR  AND "${ASN1_ROOT_DIR}" STREQUAL "")
#    message(STATUS "Can't find V2X libraries! Try to rebuild it!")
#    add_subdirectory(${LIB_V2X_DIR} ${CMAKE_CURRENT_BINARY_DIR}/v2x)
#endif()

if("${LIB_JSON_PARSER_ROOT_DIR}" STREQUAL "")
    set(LIB_JSON_PARSER_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/libjsonparse CACHE PATH "Path to the libjsonparse project sources" FORCE)
    add_subdirectory(${LIB_JSON_PARSER_ROOT_DIR} ${CMAKE_CURRENT_BINARY_DIR}/libjsonparse)
endif()

if("${LIB_GLOG_ROOT_DIR}" STREQUAL "")
    set(LIB_GLOG_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/glog CACHE PATH "Path to the glog project sources" FORCE)
    add_subdirectory(${LIB_GLOG_ROOT_DIR} ${CMAKE_CURRENT_BINARY_DIR}/glog)
endif()

if("${LIB_RESTCLIENT_ROOT_DIR}" STREQUAL "")
    set(LIB_RESTCLIENT_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/restclient-cpp CACHE PATH "Path to the restclient project sources" FORCE)
    add_subdirectory(${LIB_RESTCLIENT_ROOT_DIR} ${CMAKE_CURRENT_BINARY_DIR}/restclient)
endif()

#if(ASN1_DEBUG)
#    if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug" OR "${CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
#        add_definitions(-DSTOP_GLOG=1)
#    endif()
#endif()

link_directories(
        ${glog_BINARY_DIR}
        ${libjsonparse_BINARY_DIR}
#        ${libasn1_BINARY_DIR}/lib
        ${restclient-cpp_BINARY_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/
                    ${LIB_JSON_PARSER_ROOT_DIR}/include
                    ${LIB_GLOG_ROOT_DIR}/src
                    ${LIB_RESTCLIENT_ROOT_DIR}/include/
                    ${restclient-cpp_BINARY_DIR}/include/
#                    ${ASN1_INCLUDE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/libcluster
                    )

file(GLOB LIB_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/libcluster/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/libcluster/logger.c
    )

file(GLOB LIB_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/libcluster/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libcluster/*.hpp
    )

set(INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/libcluster/cluster.h
    ${CMAKE_CURRENT_SOURCE_DIR}/libcluster/sbc_json.h
    )

set(LINK_DIRS ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
#    ${ASN1_LIB_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/jsonparse
    ${CMAKE_CURRENT_BINARY_DIR}/glog
    ${CMAKE_CURRENT_BINARY_DIR}/restclient
    )

set(LIBRARY_NAME cluster)

find_package(Boost COMPONENTS system thread REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

link_directories(${LINK_DIRS})

add_definitions(-DBOOST_SYSTEM_NO_DEPRECATED -DBOOST_ERROR_CODE_HEADER_ONLY)
add_library(${LIBRARY_NAME} ${LIB_SOURCES} ${LIB_HEADERS})
set_property(TARGET ${LIBRARY_NAME} PROPERTY POSITION_INDEPENDENT_CODE 1)
#add_dependencies(${LIBRARY_NAME} asn1)
add_dependencies(${LIBRARY_NAME} jsonparse glog restclient-cpp)

set(LLIBS dl jsonparse glog m -pthread restclient-cpp)
#        asn1

target_link_libraries(${LIBRARY_NAME} ${LLIBS})

list(INSERT LLIBS 0 ${LIBRARY_NAME})

#Find Android log lib
find_library(log-lib log)
if(${log-lib})
    list(APPEND LLIBS ${log-lib})
endif()

add_executable(cluster-app src/main.cpp)
target_link_libraries(cluster-app ${LLIBS})
add_dependencies(cluster-app ${LIBRARY_NAME})

set(SHELL_CMD "${CMAKE_CURRENT_SOURCE_DIR}/../../tools/copy_lib_dependencies.sh ${CMAKE_CURRENT_BINARY_DIR}/cluster-app ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
add_custom_command(TARGET cluster-app
    POST_BUILD COMMAND bash -c "${SHELL_CMD}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${LIBRARY_NAME}
    )

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/conf DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_INSTALL_PREFIX /usr/local)
install(TARGETS ${LIBRARY_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
       )

install(FILES ${INCLUDES}
    DESTINATION ${CMAKE_INSTALL_PREFIX}/include
    )

set(CLUSTER_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "Path to the cluster project sources" FORCE)
set(CLUSTER_LIB_DIR ${LINK_DIRS} CACHE PATH "Path to the cluster libraries" FORCE)

if("${SBC_VERSION}" STREQUAL "")
	set(CLUSTER_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libcluster CACHE PATH "Path to the cluster header files" FORCE)
else()
	set(CLUSTER_INCLUDE_DIR /usr/local/include/sbc_${SBC_VERSION} ${CMAKE_CURRENT_SOURCE_DIR}/libcluster CACHE PATH "Path to the cluster header files" FORCE)
endif()

set(CLUSTER_LINK_LIBS ${LLIBS} CACHE PATH "Cluster libraries list" FORCE)
