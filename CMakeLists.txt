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


find_program(LCOV lcov)

if(LCOV)
    add_custom_target(generate_coverage_report
        COMMAND ${LCOV} --capture --directory ${CMAKE_SOURCE_DIR} --base-directory ${CMAKE_SOURCE_DIR} --output-file ${CMAKE_SOURCE_DIR}/coverage_raw.info
        COMMAND ${LCOV} --add-tracefile ${CMAKE_SOURCE_DIR}/coverage_raw.info --output-file ${CMAKE_SOURCE_DIR}/coverage.info
        COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_SOURCE_DIR}/coverage_raw.info
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating and merging coverage report..."
    )
endif()