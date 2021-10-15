cmake_minimum_required(VERSION 3.12)
project(Experiments)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic")

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "/Zc:preprocessor")
endif()

set(HEADERS src/experiments_component.hpp src/model.hpp src/pipeline.hpp)

set(SOURCES src/model.cpp src/pipeline.cpp src/experiments_component.cpp)

add_library(experiments ${HEADERS} ${SOURCES})
target_include_directories(experiments PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/src")
target_link_libraries(experiments PUBLIC application glm graphics voxels)
set_target_properties(experiments PROPERTIES WIN32_EXECUTABLE FALSE)
