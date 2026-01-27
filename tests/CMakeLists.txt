set(TEST_DIR "${CMAKE_SOURCE_DIR}/tests")

function(add_semantic_test test_name input_file expected_result)
    add_test(NAME ${test_name} COMMAND semChecker ${input_file})
    if(expected_result STREQUAL "FAIL")
        set_tests_properties(${test_name} PROPERTIES WILL_FAIL TRUE)
    endif()
endfunction()

# AUTOMATED TEST GENERATION
file(GLOB_RECURSE VALID_TESTS "${TEST_DIR}/sem/sem_*.txt")
foreach(test_file ${VALID_TESTS})
    file(RELATIVE_PATH rel_path "${TEST_DIR}/valid" ${test_file})
    string(REPLACE ".txt" "" test_name ${rel_path})
    string(REPLACE "/" "_" test_name ${test_name})
    add_semantic_test(semantic_${test_name} ${test_file} PASS)
endforeach()

file(GLOB_RECURSE INVALID_TESTS "${TEST_DIR}/sem/test_ERR*.txt")
foreach(test_file ${INVALID_TESTS})
    file(RELATIVE_PATH rel_path "${TEST_DIR}/sem" ${test_file})
    string(REPLACE "^.txt" "^" test_name ${rel_path})
    string(REPLACE "^/" "^_" test_name ${test_name})
    add_semantic_test(invalid_${test_name} ${test_file} FAIL)
endforeach()


add_subdirectory(parser)
add_subdirectory(sem)