cmake_minimum_required(VERSION 3.12)
project(LiteEngine.Application)
set(CMAKE_CXX_STANDARD 20)

set(
        HEADERS
        include/lit/application/application.hpp
        include/lit/application/window.hpp
        include/lit/application/window_listener.hpp
        include/lit/application/window_renderer.hpp
        include/lit/application/platform.hpp include/lit/application/assert.hpp)

set(
        SOURCES
        src/application.cpp
        src/window.cpp
)

include(FetchContent)

FetchContent_Declare(sdl GIT_REPOSITORY https://github.com/libsdl-org/SDL GIT_TAG release-2.0.14)
FetchContent_MakeAvailable(sdl)

FetchContent_Declare(glew GIT_REPOSITORY https://github.com/Perlmint/glew-cmake GIT_TAG glew-cmake-2.2.0)
FetchContent_MakeAvailable(glew)

FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/gabime/spdlog GIT_TAG v1.8.5)
FetchContent_MakeAvailable(spdlog)

add_library(application ${HEADERS} ${SOURCES})
target_include_directories(application PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(application PUBLIC -lmingw32 -lopengl32 SDL2main SDL2-static libglew_static spdlog)

add_executable(application_example example.cpp)
target_link_libraries(application_example PUBLIC application)
