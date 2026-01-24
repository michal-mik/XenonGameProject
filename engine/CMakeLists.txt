add_library(xenon_engine STATIC
    src/Engine.cpp
    src/TextureManager.cpp
)

# This tells the compiler to search in engine/include/
target_include_directories(xenon_engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# If you don't already have these here, it's fine to repeat;
# CMake will just re-use the found packages from the root.
find_package(SDL3 REQUIRED CONFIG)
find_package(box2d REQUIRED CONFIG)

target_link_libraries(xenon_engine
    PUBLIC
        SDL3::SDL3
        box2d::box2d    # if this errors later, try just 'box2d'
)

if (MSVC)
    target_compile_options(xenon_engine PRIVATE /W4 /permissive-)
else()
    target_compile_options(xenon_engine PRIVATE -Wall -Wextra -Wpedantic)
endif()

