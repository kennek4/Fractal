project(GTFO_PROFILER_CMAKE)

add_library(GTFOProfiler ${CMAKE_CURRENT_SOURCE_DIR}/gtfo_profiler.h ${CMAKE_CURRENT_SOURCE_DIR}/gtfo_profiler.cpp)
target_include_directories(GTFOProfiler PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
