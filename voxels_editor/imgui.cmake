include(FetchContent)

FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui GIT_TAG master)
FetchContent_MakeAvailable(imgui)

file(
        GLOB IMGUI_SOURCES
        "${imgui_SOURCE_DIR}/*.cpp"
        "${imgui_SOURCE_DIR}/*.h"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_sdl.cpp"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_sdl.h"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.h"
)

add_library(imgui ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC "${imgui_SOURCE_DIR}")
target_link_libraries(imgui PUBLIC -lmingw32 -lopengl32 -mwindows SDL2main SDL2-static libglew_static)