# Project Setup
cmake_minimum_required(VERSION 3.29)
project(FractalCMake LANGUAGES CXX)

set(FRACTAL_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

# C++ Standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# DIRECTORIES ##############
add_subdirectory(include)
add_subdirectory(lib/fractal)
add_subdirectory(lib/fractal/core)
add_subdirectory(lib/fractal/renderer)
add_subdirectory(lib/fractal/utility)
add_subdirectory(src)
