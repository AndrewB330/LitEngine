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

add_library(engine ${HEADERS} ${SOURCES} include/lit/engine/generators/worldgen.hpp src/engine/generators/worldgen.cpp include/lit/engine/generators/fnl.hpp include/lit/engine/systems/debug_system.hpp src/engine/systems/debug_system.cpp include/lit/engine/components/voxel_grid/voxel_grid_dense.hpp include/lit/engine/utilities/voxel_grid_utils.hpp include/lit/engine/generators/rand.hpp include/lit/engine/generators/treegen.hpp src/engine/generators/treegen.cpp include/lit/engine/components/voxel_grid/voxel_grid.hpp)
target_include_directories(engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(engine PUBLIC application common rendering EnTT::EnTT imgui)

