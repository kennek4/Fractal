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

function (add_slang_shader_target TARGET)
  cmake_parse_arguments ("SHADER" "" "" "SOURCES" ${ARGN})
    set (SHADERS_DIR ${FRACTAL_ROOT}/assets/shaders)
  set (ENTRY_POINTS -entry vertMain -entry fragMain)
  add_custom_command (
          OUTPUT ${SHADERS_DIR}
          COMMAND ${CMAKE_COMMAND} -E make_directory ${SHADERS_DIR}
  )
  add_custom_command (
          OUTPUT  ${SHADERS_DIR}/slang.spv
          COMMAND slangc ${SHADER_SOURCES} -target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name ${ENTRY_POINTS} -o slang.spv
          WORKING_DIRECTORY ${SHADERS_DIR}
          DEPENDS ${SHADERS_DIR} ${SHADER_SOURCES}
          COMMENT "Compiling Slang Shaders"
          VERBATIM
  )
  add_custom_target (${TARGET} DEPENDS ${SHADERS_DIR}/slang.spv)
endfunction()

add_slang_shader_target(FRACTAL_SHADERS SOURCES ${FRACTAL_ROOT}/assets/shaders/shader.slang)


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
            utility/FTL_Constants.h
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
add_dependencies(FractalLib FRACTAL_SHADERS)

message(STATUS "[Fractal]: Using DEBUG Libraries!")
target_link_libraries(FractalLib VulkanCppModule glfw GTFOProfiler spdlog::spdlog)
