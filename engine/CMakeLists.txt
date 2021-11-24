cmake_minimum_required(VERSION 3.12)
project(Viewer)
set(CMAKE_CXX_STANDARD 20)

set(
        HEADERS
        include/lit/viewer/viewer_app.hpp
        include/lit/viewer/viewer_window.hpp
        include/lit/engine/scene.hpp
        include/lit/engine/components/voxel_grid/voxel_grid_base.hpp
        include/lit/engine/components/voxel_grid/voxel_grid_dense.hpp
        include/lit/engine/components/voxel_grid/voxel_grid_sparse.hpp
        include/lit/engine/components/voxel_grid/voxel_grid_octree.hpp
        include/lit/engine/resources_manager.hpp
        include/lit/engine/systems/voxels/voxel_renderer.hpp
        include/lit/engine/components/camera.hpp
        include/lit/engine/systems/observer_input_controller.hpp
        include/lit/engine/systems/system.hpp
        include/lit/engine/components/tag.hpp
        include/lit/engine/utilities/allocator.hpp
        include/lit/engine/utilities/fenwick_tree.hpp
        include/lit/viewer/debug_window.hpp
        include/lit/engine/systems/voxels/voxel_world_gpu_data_manager.hpp 
		include/lit/engine/systems/voxels/voxel_world_generator.hpp 
		include/lit/engine/components/sky_box.hpp 
		include/lit/engine/systems/renderers/sky_box_renderer.hpp 
		include/lit/engine/systems/renderers/camera_pre_renderer.hpp 
		include/lit/engine/systems/renderers/tone_mapping_renderer.hpp include/lit/engine/components/voxel_grid/voxel_grid_sparse_lod_data.hpp)

set(
        SOURCES
        src/viewer/viewer_app.cpp
        src/viewer/viewer_window.cpp
        src/engine/scene.cpp
        src/engine/resources_manager.cpp
        src/engine/systems/player_controller.cpp
        src/engine/systems/voxels/voxel_renderer.cpp
        src/engine/components/camera.cpp
        src/engine/algorithms/allocator.cpp
        src/engine/algorithms/fenwick_tree.cpp
        src/viewer/debug_window.cpp
        src/engine/components/voxel_tree.cpp
        src/engine/systems/voxels/voxel_world_gpu_data_manager.cpp
		src/engine/systems/voxels/voxel_world_generator.cpp
		src/engine/components/sky_box.cpp
		src/engine/systems/sky_box_renderer.cpp
		src/engine/systems/camera_pre_renderer.cpp
		src/engine/systems/tone_mapping_renderer.cpp
		)

include(imgui.cmake)

add_library(engine ${HEADERS} ${SOURCES})
target_include_directories(engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(engine PUBLIC application common rendering EnTT::EnTT imgui)

