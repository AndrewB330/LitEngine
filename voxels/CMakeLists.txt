cmake_minimum_required(VERSION 3.12)
project(LiteEngine.VoxelWorld)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic")

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS "/Zc:preprocessor")
endif()

set(
        HEADERS
        src/debug_ui.hpp
        src/game_renderer.hpp
        src/controls/player_controller.hpp
        include/lit/voxels/voxel_grid.hpp
        include/lit/voxels/voxel_object.hpp
        include/lit/voxels/voxel_world.hpp
        src/gpu_data_managers/voxel_data_manager.hpp
        src/game_rendering/shaders/ray_tracing.hpp
        src/game_rendering/shaders/lighting.hpp
        src/game_rendering/shaders/taa.hpp
        src/game_rendering/shaders/debug.hpp
        src/game_rendering/shaders/denoising.hpp
        src/game_rendering/shaders/final.hpp
        src/game_rendering/shaders/light_mixer.hpp
        src/gpu_data_managers/sky_box_loader.hpp
        src/debug_options.hpp
        src/game_rendering/shaders/copy.hpp
        src/game_rendering/shaders/reflections.hpp
        src/game_rendering/voxels_pipeline.hpp
        src/game_rendering/camera.hpp include/lit/voxels/voxel_grid_2.hpp include/lit/voxels/voxel_grid_with_lod.hpp)

set(
        SOURCES
        src/debug_ui.cpp
        src/game_renderer.cpp
        src/controls/player_controller.cpp
        src/game_common/voxel_grid.cpp
        src/game_common/voxel_object.cpp
        src/game_common/voxel_world.cpp
        src/gpu_data_managers/voxel_data_manager.cpp
        src/gpu_data_managers/sky_box_loader.cpp
        src/game_rendering/voxels_pipeline.cpp
        src/game_rendering/camera.cpp)

include(../voxels_editor/imgui.cmake)

add_library(voxels ${HEADERS} ${SOURCES})
target_include_directories(voxels PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(voxels PUBLIC application imgui glm graphics engine)
set_target_properties(voxels PROPERTIES WIN32_EXECUTABLE FALSE)
