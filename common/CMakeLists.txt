cmake_minimum_required(VERSION 3.12)
project(Common)
set(CMAKE_CXX_STANDARD 20)

set(
        HEADERS
        include/lit/common/time_utils.hpp
        include/lit/common/algorithms/fenwick_tree.hpp
        include/lit/common/glm_ext/comparators.hpp
        include/lit/common/glm_ext/region.hpp
        include/lit/common/images/lodepng.hpp
        include/lit/common/images/images.hpp
)

set(
        SOURCES
        src/common/time_utils.cpp
        src/common/algorithms/fenwick_tree.cpp
        src/common/images/lodepng.cpp
        src/common/images/images.cpp
)

include(FetchContent)

if (NOT TARGET glm)
    FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm GIT_TAG master)
    FetchContent_MakeAvailable(glm)
endif ()

add_library(common ${HEADERS} ${SOURCES})
target_include_directories(common PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(common glm)

add_executable(common_example example.cpp)
target_link_libraries(common_example common)