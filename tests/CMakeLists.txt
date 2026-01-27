set(TEST_DIR "${CMAKE_SOURCE_DIR}/tests")

function(add_semantic_test test_name input_file expected_result)
    add_test(NAME ${test_name} COMMAND semChecker ${input_file})
    if(expected_result STREQUAL "FAIL")
        set_tests_properties(${test_name} PROPERTIES WILL_FAIL TRUE)
    endif()
endfunction()

# AUTOMATED TEST GENERATION
file(GLOB_RECURSE VALID_TESTS "${TEST_DIR}/valid/sem_*.txt")
foreach(test_file ${VALID_TESTS})
    file(RELATIVE_PATH rel_path "${TEST_DIR}/valid" ${test_file})
    string(REPLACE ".txt" "" test_name ${rel_path})
    string(REPLACE "/" "_" test_name ${test_name})
    add_semantic_test(semantic_${test_name} ${test_file} PASS)
endforeach()

file(GLOB_RECURSE INVALID_TESTS "${TEST_DIR}/invalid/*.txt")
foreach(test_file ${INVALID_TESTS})
    file(RELATIVE_PATH rel_path "${TEST_DIR}/invalid" ${test_file})
    string(REPLACE "^.txt" "^" test_name ${rel_path})
    string(REPLACE "^/" "^_" test_name ${test_name})
    add_semantic_test(invalid_${test_name} ${test_file} FAIL)
endforeach()


add_subdirectory(valid)

find_program(LCOV lcov)

if(LCOV)
    add_custom_target(coverage
        COMMAND ${LCOV} --capture --directory ${CMAKE_BINARY_DIR} --base-directory ${CMAKE_SOURCE_DIR} --output-file ${CMAKE_SOURCE_DIR}/coverage_raw.info
        COMMAND ${LCOV} --add-tracefile ${CMAKE_SOURCE_DIR}/coverage_raw.info --output-file ${CMAKE_SOURCE_DIR}/coverage.info
        COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_SOURCE_DIR}/coverage_raw.info
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating and merging coverage report..."
    )
endif()