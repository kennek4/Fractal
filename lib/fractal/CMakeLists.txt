project(FractalLib)

include(core/CMakeLists.txt)
include(renderer/CMakeLists.txt)
include(utility/CMakeLists.txt)

add_library(VulkanCppModule INTERFACE)
add_library(Vulkan::cppm ALIAS VulkanCppModule)
target_link_libraries(VulkanCppModule INTERFACE Vulkan::Vulkan)
target_compile_definitions(VulkanCppModule
    INTERFACE VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1 VULKAN_HPP_NO_STRUCT_CONSTRUCTORS=1
)

set_target_properties(VulkanCppModule PROPERTIES CXX_STANDARD 20)

add_library(FractalLib STATIC)
target_sources(FractalLib 
    PRIVATE # SRC FILES
        ${CORE_SRC}
        ${RENDERER_SRC}
        ${UTILITY_SRC}

    PRIVATE # HEADER FILES
        FILE_SET internalHeaders
        TYPE HEADERS#
        BASE_DIRS
            core
            renderer
            utility
        FILES
            core/FTL_Application.h 
            core/FTL_Window.h 
            renderer/FTL_Renderer.h 
            utility/FTL_Log.h
            utility/FTL_Types.h
            utility/FTL_pch.h

    PUBLIC
        FILE_SET consumerHeaders
        TYPE HEADERS
        BASE_DIRS
            ${PROJECT_SOURCE_DIR}
        FILES
            Fractal.h
)

target_precompile_headers(FractalLib PRIVATE utility/FTL_pch.h)

message(STATUS "[Fractal]: Using DEBUG Libraries!")
target_link_libraries(FractalLib VulkanCppModule glfw GTFOProfiler spdlog::spdlog)
