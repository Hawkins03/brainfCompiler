cmake_minimum_required(VERSION 3.10)
project(bfParser C)

# Set globals and build type
set(CMAKE_C_STANDARD 99)
set(CMAKE_BUILD_TYPE "Debug")

# Enable testing
include(CTest)
enable_testing()

#compiler options
add_compile_options(-g -Wall -O0 --coverage)
add_link_options(--coverage)

# Add subdirectories
add_subdirectory(src)
add_subdirectory(tests)