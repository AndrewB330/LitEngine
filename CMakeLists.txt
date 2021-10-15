cmake_minimum_required(VERSION 3.12)
project(LitEngine)
set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic")

include(FetchContent)
FetchContent_Declare(application GIT_REPOSITORY https://github.com/LitEngine/Application GIT_TAG master)
FetchContent_MakeAvailable(application)

add_subdirectory(common)
add_subdirectory(graphics)
add_subdirectory(engine)
add_subdirectory(voxels)

add_executable(lit_engine runnable.cpp)
target_link_libraries(lit_engine PUBLIC voxels application graphics engine)
