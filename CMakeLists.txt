cmake_minimum_required(VERSION 3.10)
project(bfParser C)

# 1. SET GLOBALS AND BUILD TYPE FIRST
set(CMAKE_C_STANDARD 99)
set(CMAKE_BUILD_TYPE "Debug")

# Flags for code coverage - Applied globally for simplicity
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -Wall --coverage")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")

include(CTest)
enable_testing()

# 2. DEFINE SOURCE GROUPS
set(HEADERS
    include/exp.h
    include/interp.h
    include/ir.h
    include/parser.h
    include/reader.h
    include/semantics.h
    include/stmt.h
    include/structs.h
    include/utils.h
)

set(SOURCES
    src/exp.c   
    src/interp.c 
    src/parser.c
    src/reader.c
    src/stmt.c
    src/semantics.c
    src/utils.c
)

# 3. DEFINE EXECUTABLES
add_executable(parser src/parse_file.c ${SOURCES} ${HEADERS})
add_executable(semChecker src/check_semantics.c ${SOURCES} ${HEADERS})

target_include_directories(parser PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_include_directories(semChecker PRIVATE ${CMAKE_SOURCE_DIR}/include)

# 4. COVERAGE TARGET (LCOV)
find_program(LCOV lcov)
find_program(GENHTML genhtml)

if(LCOV AND GENHTML)
    add_custom_target(coverage
        # Use a single shell string to ensure the full sequence runs in WSL
        COMMAND ${LCOV} --capture --directory ${CMAKE_SOURCE_DIR} --base-directory ${CMAKE_SOURCE_DIR} --output-file ${CMAKE_SOURCE_DIR}/coverage.info
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating report in WSL... ignoring test exit codes to ensure lcov runs."
    )
endif() 

# 5. TEST DEFINITIONS
set(TEST_DIR "${CMAKE_SOURCE_DIR}/tests")

function(add_parser_test test_name input_file expected_result)
    add_test(NAME ${test_name} COMMAND parser ${input_file})
    if(expected_result STREQUAL "FAIL")
        set_tests_properties(${test_name} PROPERTIES WILL_FAIL TRUE)
    endif()
endfunction()

function(add_semantic_test test_name input_file expected_result)
    add_test(NAME ${test_name} COMMAND semChecker ${input_file})
    if(expected_result STREQUAL "FAIL")
        set_tests_properties(${test_name} PROPERTIES WILL_FAIL TRUE)
    endif()
endfunction()

# 6. AUTOMATED TEST GENERATION
file(GLOB_RECURSE VALID_TESTS "${TEST_DIR}/valid/*.txt")
foreach(test_file ${VALID_TESTS})
    file(RELATIVE_PATH rel_path "${TEST_DIR}/valid" ${test_file})
    string(REPLACE ".txt" "" test_name ${rel_path})
    string(REPLACE "/" "_" test_name ${test_name})
    
    # Check if it's a semantic test based on filename prefix
    if(test_name MATCHES "^sem_")
        add_semantic_test(semantic_valid_${test_name} ${test_file} PASS)
    endif()
    add_parser_test(valid_${test_name} ${test_file} PASS)
endforeach()

# I DON'T CARE HOW STUPID THIS LOOKS. IT TOOK HOURS. DO NOT TOUCH!!!!!!!!
add_test(NAME post_test_coverage COMMAND make coverage)

#file(GLOB_RECURSE INVALID_TESTS "${TEST_DIR}/invalid/*.txt")
#foreach(test_file ${INVALID_TESTS})
#    file(RELATIVE_PATH rel_path "${TEST_DIR}/invalid" ${test_file})
#    string(REPLACE ".txt" "" test_name ${rel_path})
#    string(REPLACE "/" "_" test_name ${test_name})
#    add_semantic_test(invalid_${test_name} ${test_file} FAIL)
#endforeach()
