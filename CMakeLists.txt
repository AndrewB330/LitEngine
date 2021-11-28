cmake_minimum_required(VERSION 3.12)
project(LitEngine)
set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic")

macro(append_cxx_flag text)
    foreach (flag
            CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)

        string(APPEND ${flag} " ${text}")

    endforeach()
endmacro()

# add_definitions(-DHAVE_LIBC)
append_cxx_flag("/std:c++latest")
append_cxx_flag("/Zc:preprocessor")
append_cxx_flag("/wd4103")
append_cxx_flag("/openmp")

include(FetchContent)

FetchContent_Declare(entt GIT_REPOSITORY https://github.com/skypjack/entt GIT_TAG master)
FetchContent_MakeAvailable(entt)

add_subdirectory(application)
add_subdirectory(common)
add_subdirectory(rendering)
add_subdirectory(engine)

add_executable(lit_engine runnable.cpp)
target_link_libraries(lit_engine PUBLIC engine)

