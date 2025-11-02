# PROJECT SETUP
cmake_minimum_required(VERSION 3.29)
project(FractalCMake LANGUAGES CXX)

set(FRACTAL_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

# C++ STANDARD
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

include(FetchContent)

# DEPENDENCIES
find_package(Vulkan QUIET REQUIRED)
if (NOT Vulkan_FOUND)
    message(FATAL_ERROR "[Fractal] Vulkan was not found with the 'find_package()' command.")
endif()

find_package(glfw3 3.4 QUIET REQUIRED)
if (NOT TARGET glfw3)
    message(STATUS "[Fractal] Using FetchContent for glfw3.")
    FetchContent_Declare(
        GIT_REPOSITORY https://github.com/glfw/glfw.git 
        GIT_TAG 3.4
    )
endif()

find_package(glm QUIET REQUIRED)
if (NOT TARGET glm)
    message(STATUS "[Fractal] Using FetchContent for glm.")
    FetchContent_Declare(
        GIT_REPOSITORY https://github.com/g-truc/glm.git 
        GIT_TAG 1.0.2
    )
endif()



# BUILD TYPE
message(STATUS "[Fractal] CMake Generator: ${CMAKE_GENERATOR}")
if (CMAKE_BUILD_TYPE STREQUAL Debug)
    message(STATUS "[Fractal]: Building Debug...")
    add_compile_definitions(__FRACTAL_BUILD_DEBUG)

    find_package(spdlog REQUIRED)
    if (NOT TARGET spdlog) 
        message("[Fractal] Using FetchContent for spdlog.")
        FetchContent_Declare(
            GIT_REPOSITORY https://github.com/gabime/spdlog.git
            GIT_TAG v1.16.0
        )
    endif()

else()
    message(STATUS "[Fractal]: Building Release...")
    add_compile_definitions(__FRACTAL_BUILD_RELEASE)
endif()

# SYSTEM TYPE
if (UNIX AND NOT APPLE)
    message(STATUS "[Fractal]: Hello, Linux! :)")
    add_compile_definitions(__FRACTAL_PLATFORM_LINUX)
elseif(WIN32)
    message(STATUS "[Fractal]: Hello, Windows! :)")
    add_compile_definitions(__FRACTAL_PLATFORM_WIN32)
else()
    message(STATUS "[Fractal]: Hello..?")
    add_compile_definitions(__FRACTAL_PLATFORM_UNSUPPORTED)
endif()

# DIRECTORIES ##############
add_subdirectory(include/gtfoprofiler)
add_subdirectory(lib/fractal)
add_subdirectory(src)
