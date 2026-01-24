cmake_minimum_required(VERSION 3.26)
project(AGPT_Project1 LANGUAGES C CXX)

# C standard for Box2D, C++ standard for your engine/game
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find SDL3 and Box2D installed via Homebrew.
# If CMake complains about target names, see note below.
find_package(SDL3 REQUIRED CONFIG)
find_package(box2d REQUIRED CONFIG)

add_subdirectory(engine)
add_subdirectory(game)
