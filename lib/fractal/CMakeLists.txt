project(FractalLib)

set(FRACTAL_HEADERS ${CORE_HEADERS} ${RENDERER_HEADERS}
                    ${UTILITY_HEADERS})
set(FRACTAL_SRC ${CORE_SRC} ${RENDERER_SRC} ${UTILITY_SRC})

add_library(FractalLib)
target_sources(FractalLib
    PRIVATE # SRC FILES
        ${FRACTAL_SRC} 

    PRIVATE
        FILE_SET internalHeaders
        TYPE HEADERS
        BASE_DIRS
            core
            renderer
            utility
        FILES
            ${FRACTAL_HEADERS}

    INTERFACE
        FILE_SET consumerHeaders
        TYPE HEADERS
        FILES
            Fractal.H

    PUBLIC
        Fractal.cpp
)
