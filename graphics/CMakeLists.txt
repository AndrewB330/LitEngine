cmake_minimum_required(VERSION 3.19)
project(main)

set(CMAKE_CXX_STANDARD 20)

set(
        HEADERS
        include/lit/gl/shader.hpp
        include/lit/gl/program.hpp
        include/lit/gl/frame_buffer.hpp
        include/lit/gl/texture.hpp
        include/lit/gl/shader.hpp
        include/lit/rendering/pipeline_node.hpp
        include/lit/rendering/cpp_shader_utils.hpp 
        include/lit/gl/cubemap.hpp 
        include/lit/gl/context.hpp
        include/lit/gl/assert.hpp include/lit/gl/vertex_buffer.hpp)

set(
        SOURCES
        src/gl/shader.cpp
        src/gl/program.cpp
        src/gl/texture.cpp
        src/gl/frame_buffer.cpp
        src/gl/shader.cpp
        src/rendering/cpp_shader.cpp
        src/rendering/pipeline_node.cpp 
        src/gl/cubemap.cpp 
        src/gl/context.cpp
        src/gl/vertex_buffer.cpp)

include(FetchContent)

if (NOT TARGET glm)
    FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm GIT_TAG 0.9.9.8)
    FetchContent_MakeAvailable(glm)
endif()

if (NOT TARGET spdlog)
    FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/gabime/spdlog GIT_TAG v1.8.5)
    FetchContent_MakeAvailable(spdlog)
endif()

if (NOT TARGET liglew_static)
    FetchContent_Declare(glew GIT_REPOSITORY https://github.com/Perlmint/glew-cmake GIT_TAG glew-cmake-2.2.0)
    FetchContent_MakeAvailable(glew)
endif()

add_library(graphics ${HEADERS} ${SOURCES})
target_include_directories(graphics PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(graphics glm common spdlog libglew_static -lopengl32)

add_executable(graphics_run run.cpp)
target_link_libraries(graphics_run graphics)