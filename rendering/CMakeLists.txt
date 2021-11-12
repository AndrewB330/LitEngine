cmake_minimum_required(VERSION 3.12)
project(Rendering)
set(CMAKE_CXX_STANDARD 20)

set(
        HEADERS
        include/lit/rendering/opengl/api.hpp
        include/lit/rendering/opengl/shader.hpp
        include/lit/rendering/opengl/texture.hpp
        include/lit/rendering/opengl/vertex_array.hpp
        include/lit/rendering/opengl/frame_buffer.hpp
        include/lit/rendering/opengl/utils.hpp include/lit/rendering/opengl/uniform_buffer.hpp)

set(
        SOURCES
        src/opengl/api.cpp
        src/opengl/shader.cpp
        src/opengl/texture.cpp
        src/opengl/vertex_array.cpp
        src/opengl/frame_buffer.cpp
        src/opengl/uniform_buffer.cpp)

add_library(rendering ${HEADERS} ${SOURCES})
target_include_directories(rendering PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(rendering PRIVATE glm spdlog common libglew_static imgui)