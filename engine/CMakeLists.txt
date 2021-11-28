cmake_minimum_required(VERSION 3.12)
project(Viewer)
set(CMAKE_CXX_STANDARD 20)

file(
        GLOB_RECURSE HEADERS
        "*.hpp"
)

file(
        GLOB_RECURSE SOURCES
        "*.cpp"
)

include(imgui.cmake)

add_library(engine ${HEADERS} ${SOURCES})
target_include_directories(engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(engine PUBLIC application common rendering EnTT::EnTT imgui)

