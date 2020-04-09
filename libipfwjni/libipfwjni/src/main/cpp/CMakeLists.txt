cmake_minimum_required(VERSION 3.4.1)

set( CMAKE_CXX_STANDARD 11 )

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../../3rdparty/spdlog/
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../../../src/
)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../../../../build/linux/CMakeLists.txt)

add_library( ipfwjni SHARED
        ${CMAKE_CURRENT_LIST_DIR}/jni.cpp
        ${CMAKE_CURRENT_LIST_DIR}/IpFw.cpp
        )

find_library(
        log-lib
        log)
target_link_libraries(
        ipfwjni
        ipfw
        ${log-lib})


