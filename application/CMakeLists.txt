cmake_minimum_required(VERSION 3.12)
project(LiteEngine.Application)
set(CMAKE_CXX_STANDARD 20)

set(
        HEADERS
        include/lit/application/application.hpp
        include/lit/application/window.hpp
        include/lit/application/window_listener.hpp
        include/lit/application/window_renderer.hpp
)

set(
        SOURCES
        src/application.cpp
        src/window.cpp
)

include(FetchContent)

if (NOT TARGET glew)
    FetchContent_Declare(glew GIT_REPOSITORY https://github.com/LitEngine/glew GIT_TAG master)
    FetchContent_MakeAvailable(glew)
endif ()

if (NOT TARGET sdl2)
    FetchContent_Declare(sdl2 GIT_REPOSITORY https://github.com/LitEngine/SDL2 GIT_TAG master)
    FetchContent_MakeAvailable(sdl2)
endif ()

add_library(application ${HEADERS} ${SOURCES})
target_include_directories(application PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(application PUBLIC common glew sdl2)

add_executable(application_example example.cpp)
target_link_libraries(application_example PUBLIC application)
