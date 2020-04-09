cmake_minimum_required(VERSION 2.8)
project(ipfw)

include(FindBoost)
find_package(Boost REQUIRED COMPONENTS  program_options)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/
)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../build/linux/CMakeLists.txt)

ADD_EXECUTABLE(ipfw_test main.cpp)
TARGET_LINK_LIBRARIES(ipfw_test ipfw ${Boost_LIBRARIES})
