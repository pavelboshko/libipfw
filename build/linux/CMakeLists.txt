cmake_minimum_required(VERSION 2.8)
project(ipfw)
find_package(Threads)

set( CMAKE_CXX_STANDARD 11 )

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/../../../3rdparty/boost-mobile/
    ${CMAKE_CURRENT_LIST_DIR}/../../../3rdparty/spdlog/include/
)

#SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DDEBUG")

ADD_LIBRARY(ipfw SHARED
    ${CMAKE_CURRENT_LIST_DIR}/../../src/tun.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/ip_utils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/tcp_packet.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/tcp_session.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/tcp_session_fsm.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/log.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/connection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/ipfw.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../../src/ipfw_tun.cpp
)
TARGET_LINK_LIBRARIES(ipfw ${CMAKE_THREAD_LIBS_INIT})
