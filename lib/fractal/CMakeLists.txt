project(FractalLib)

include(core/CMakeLists.txt)
include(renderer/CMakeLists.txt)
include(utility/CMakeLists.txt)

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
            renderer/FTL_VulkanCore.h 
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


target_link_libraries(FractalLib ${Vulkan_LIBRARIES} glfw)
