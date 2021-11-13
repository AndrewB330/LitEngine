cmake_minimum_required(VERSION 3.12)
project(Viewer)
set(CMAKE_CXX_STANDARD 20)

set(
        HEADERS
        include/lit/viewer/viewer_app.hpp
        include/lit/viewer/viewer_window.hpp
        include/lit/engine/scene.hpp
        include/lit/engine/components/voxel_grid.hpp
        include/lit/engine/resources_manager.hpp
        include/lit/engine/systems/voxels/voxel_renderer.hpp
        include/lit/engine/components/camera.hpp
        include/lit/engine/systems/observer_input_controller
        include/lit/engine/systems/system.hpp
        include/lit/engine/entity_view.hpp
        include/lit/engine/components/tag.hpp
        include/lit/engine/algorithms/allocator.hpp
        include/lit/engine/algorithms/fenwick_tree.hpp
        include/lit/viewer/debug_window.hpp
        include/lit/engine/components/voxel_tree.hpp
        include/lit/engine/components/voxel_world.hpp
        include/lit/engine/systems/voxels/voxel_world_gpu_data_manager.hpp)

set(
        SOURCES
        src/viewer/viewer_app.cpp
        src/engine/components/voxel_grid.cpp
        src/viewer/viewer_window.cpp
        src/engine/scene.cpp
        src/engine/resources_manager.cpp
        src/engine/systems/player_controller.cpp
        src/engine/systems/voxels/voxel_renderer.cpp
        src/engine/components/camera.cpp
        src/engine/entity_view.cpp
        src/engine/algorithms/allocator.cpp
        src/engine/algorithms/fenwick_tree.cpp
        src/viewer/debug_window.cpp
        src/engine/components/voxel_tree.cpp
        src/engine/components/voxel_world.cpp
        src/engine/systems/voxels/voxel_world_gpu_data_manager.cpp)

include(imgui.cmake)

add_library(engine ${HEADERS} ${SOURCES})
target_include_directories(engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(engine PUBLIC application common rendering EnTT::EnTT imgui)

