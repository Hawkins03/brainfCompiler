cmake_minimum_required(VERSION 3.10)
project(bfParser C)

# 1. SET GLOBALS AND BUILD TYPE FIRST
set(CMAKE_C_STANDARD 99)
set(CMAKE_BUILD_TYPE "Debug")

# Flags for code coverage - Applied globally for simplicity
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -Wall --coverage -fprofile-arcs -ftest-coverage")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage -fprofile-arcs -ftest-coverage")

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

# 5. TEST DEFINITIONS
set(TEST_DIR "${CMAKE_SOURCE_DIR}/tests")

function(add_semantic_test test_name input_file expected_result)
    add_test(NAME ${test_name} COMMAND semChecker ${input_file})
    if(expected_result STREQUAL "FAIL")
        set_tests_properties(${test_name} PROPERTIES WILL_FAIL TRUE)
    endif()
endfunction()

# 6. AUTOMATED TEST GENERATION
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
    string(REPLACE ".txt" "" test_name ${rel_path})
    string(REPLACE "/" "_" test_name ${test_name})
    add_semantic_test(invalid_${test_name} ${test_file} FAIL)
endforeach()

add_test(NAME parser_atomic1 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/atomic1.txt)
set_tests_properties( parser_atomic1 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_STR\(x\);.*")
add_test(NAME parser_atomic2 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/atomic2.txt)
set_tests_properties( parser_atomic2 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_STR\(abc\);.*")
add_test(NAME parser_atomic3 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/atomic3.txt)
set_tests_properties( parser_atomic3 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_STR\(x1\);.*")
add_test(NAME parser_atomic4 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/atomic4.txt)
set_tests_properties( parser_atomic4 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_NUM\(123\);.*")
add_test(NAME parser_atomic5 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/atomic5.txt)
set_tests_properties( parser_atomic5 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_UNARY\(!, STR\(x\)\);.*")
add_test(NAME parser_atomic6 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/atomic6.txt)
set_tests_properties( parser_atomic6 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_UNARY\(~, STR\(x\)\);.*")
add_test(NAME parser_atomic7 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/atomic7.txt)
set_tests_properties( parser_atomic7 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_UNARY\(\+\+, STR\(x\)\);.*")
add_test(NAME parser_atomic8 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/atomic8.txt)
set_tests_properties( parser_atomic8 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_UNARY\(--, STR\(x\)\);.*")
add_test(NAME parser_atomic9 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/atomic9.txt)
set_tests_properties( parser_atomic9 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_UNARY\(-, STR\(x\)\);.*")
add_test(NAME parser_atomic10 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/atomic10.txt)
set_tests_properties( parser_atomic10 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_UNARY\(-, UNARY\(STR\(x\), \+\+\)\);.*")
add_test(NAME parser_atomic11 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/atomic11.txt)
set_tests_properties( parser_atomic11 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_NUM\(97\);.*")
add_test(NAME parser_atomic12 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/atomic12.txt)
set_tests_properties( parser_atomic12 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_NUM\(49\);.*")
add_test(NAME parser_atomic13 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/atomic13.txt)
set_tests_properties( parser_atomic13 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_NUM\(33\);.*")
add_test(NAME parser_atomic14 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/atomic14.txt)
set_tests_properties( parser_atomic14 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_NUM\(32\);.*")
add_test(NAME parser_atomic15 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/atomic15.txt)
set_tests_properties( parser_atomic15 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_NUM\(0\);.*")

add_test(NAME parser_true COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/true1.txt)
set_tests_properties( parser_true PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_NUM\(1\);.*")
add_test(NAME parser_false COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/false1.txt)
set_tests_properties( parser_false PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_NUM\(0\);.*")

add_test(NAME parser_binary1 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/binary1.txt)
set_tests_properties( parser_binary1 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), \+, STR\(y\)\);.*")
add_test(NAME parser_binary2 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/binary2.txt)
set_tests_properties( parser_binary2 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), -, STR\(y\)\);.*")
add_test(NAME parser_binary3 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/binary3.txt)
set_tests_properties( parser_binary3 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), \*, STR\(y\)\);.*")
add_test(NAME parser_binary4 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/binary4.txt)
set_tests_properties( parser_binary4 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), /, STR\(y\)\);.*")
add_test(NAME parser_binary5 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/binary5.txt)
set_tests_properties( parser_binary5 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), %, STR\(y\)\);.*")
add_test(NAME parser_binary6 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/binary6.txt)
set_tests_properties( parser_binary6 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), |, STR\(y\)\);.*")
add_test(NAME parser_binary7 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/binary7.txt)
set_tests_properties( parser_binary7 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), ^, STR\(y\)\);.*")
add_test(NAME parser_binary8 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/binary8.txt)
set_tests_properties( parser_binary8 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), &, STR\(y\)\);.*")
add_test(NAME parser_binary9 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/binary9.txt)
set_tests_properties( parser_binary9 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), <<, STR\(y\)\);.*")
add_test(NAME parser_binary10 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/binary10.txt)
set_tests_properties( parser_binary10 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), >>, STR\(y\)\);.*")
add_test(NAME parser_binary11 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/binary11.txt)
set_tests_properties( parser_binary11 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), <, STR\(y\)\);.*")
add_test(NAME parser_binary12 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/binary12.txt)
set_tests_properties( parser_binary12 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), <=, STR\(y\)\);.*")
add_test(NAME parser_binary13 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/binary13.txt)
set_tests_properties( parser_binary13 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), >, STR\(y\)\);.*")
add_test(NAME parser_binary14 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/binary14.txt)
set_tests_properties( parser_binary14 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), >=, STR\(y\)\);.*")
add_test(NAME parser_binary15 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/binary15.txt)
set_tests_properties( parser_binary15 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), ==, STR\(y\)\);.*")
add_test(NAME parser_binary16 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/binary16.txt)
set_tests_properties( parser_binary16 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), !=, STR\(y\)\);.*")
add_test(NAME parser_binary17 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/binary17.txt)
set_tests_properties( parser_binary17 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), &&, STR\(y\)\);.*")
add_test(NAME parser_binary18 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/binary18.txt)
set_tests_properties( parser_binary18 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), ||, STR\(y\)\);.*")
add_test(NAME parser_binary19 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/binary19.txt)
set_tests_properties( parser_binary19 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), =, STR\(y\)\);.*")
add_test(NAME parser_binary20 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/binary20.txt)
set_tests_properties( parser_binary20 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), \+=, STR\(y\)\);.*")
add_test(NAME parser_binary21 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/binary21.txt)
set_tests_properties( parser_binary21 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), -=, STR\(y\)\);.*")
add_test(NAME parser_binary22 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/binary22.txt)
set_tests_properties( parser_binary22 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), \*=, STR\(y\)\);.*")
add_test(NAME parser_binary23 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/binary23.txt)
set_tests_properties( parser_binary23 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), /=, STR\(y\)\);.*")
add_test(NAME parser_binary24 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/binary24.txt)
set_tests_properties( parser_binary24 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), %=, STR\(y\)\);.*")
add_test(NAME parser_binary25 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/binary25.txt)
set_tests_properties( parser_binary25 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), <<=, STR\(y\)\);.*")
add_test(NAME parser_binary26 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/binary26.txt)
set_tests_properties( parser_binary26 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), >>=, STR\(y\)\);.*")
add_test(NAME parser_binary27 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/binary27.txt)
set_tests_properties( parser_binary27 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), &=, STR\(y\)\);.*")
add_test(NAME parser_binary28 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/binary28.txt)
set_tests_properties( parser_binary28 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), ^=, STR\(y\)\);.*")
add_test(NAME parser_binary29 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/binary29.txt)
set_tests_properties( parser_binary29 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), |=, STR\(y\)\);.*")

add_test(NAME parser_prec1 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/prec1.txt)
set_tests_properties( parser_prec1 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), \+, OP\(STR\(y\), \*, STR\(z\)\)\);.*")
add_test(NAME parser_prec2 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/prec2.txt)
set_tests_properties( parser_prec2 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(OP\(STR\(x\), \*, STR\(y\)\), \+, STR\(z\)\);.*")
add_test(NAME parser_prec3 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/prec3.txt)
set_tests_properties( parser_prec3 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(OP\(STR\(x\), \+, STR\(y\)\), <, STR\(z\)\);.*")
add_test(NAME parser_prec4 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/prec4.txt)
set_tests_properties( parser_prec4 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), <, OP\(STR\(y\), \+, STR\(z\)\)\);.*")
add_test(NAME parser_prec5 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/prec5.txt)
set_tests_properties( parser_prec5 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(OP\(STR\(x\), >, STR\(y\)\), ==, STR\(z\)\);.*")
add_test(NAME parser_prec6 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/prec6.txt)
set_tests_properties( parser_prec6 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), ==, OP\(STR\(y\), >, STR\(z\)\)\);.*")
add_test(NAME parser_prec7 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/prec7.txt)
set_tests_properties( parser_prec7 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(OP\(STR\(x\), ==, STR\(y\)\), ==, STR\(z\)\);.*")
add_test(NAME parser_prec8 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/prec8.txt)
set_tests_properties( parser_prec8 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(OP\(STR\(x\), &&, STR\(y\)\), ||, STR\(z\)\);.*")
add_test(NAME parser_prec9 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/prec9.txt)
set_tests_properties( parser_prec9 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), ||, OP\(STR\(y\), &&, STR\(z\)\)\);.*")
add_test(NAME parser_prec10 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/prec10.txt)
set_tests_properties( parser_prec10 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(OP\(OP\(OP\(OP\(STR\(a\), \+, OP\(STR\(b\), \*, STR\(c\)\)\), <, STR\(d\)\), ==, STR\(e\)\), &&, STR\(f\)\), ||, STR\(g\)\);.*")
add_test(NAME parser_prec11 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/prec11.txt)
set_tests_properties( parser_prec11 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(OP\(OP\(OP\(OP\(STR\(a\), \*, STR\(b\)\), \+, STR\(c\)\), <, STR\(d\)\), &&, OP\(STR\(e\), ==, STR\(f\)\)\), ||, STR\(g\)\);.*")
add_test(NAME parser_prec12 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/prec12.txt)
set_tests_properties( parser_prec12 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(OP\(STR\(a\), -, STR\(b\)\), -, STR\(c\)\);.*")
add_test(NAME parser_prec13 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/prec13.txt)
set_tests_properties( parser_prec13 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(OP\(STR\(a\), /, STR\(b\)\), /, STR\(c\)\);.*")
add_test(NAME parser_prec14 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/prec14.txt)
set_tests_properties( parser_prec14 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(OP\(STR\(a\), &&, STR\(b\)\), &&, STR\(c\)\);.*")
add_test(NAME parser_prec15 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/prec15.txt)
set_tests_properties( parser_prec15 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(OP\(STR\(a\), ||, STR\(b\)\), ||, STR\(c\)\);.*")
add_test(NAME parser_prec16 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/prec16.txt)
set_tests_properties( parser_prec16 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(a\), =, OP\(STR\(b\), =, STR\(c\)\)\);.*")
add_test(NAME parser_prec17 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/prec17.txt)
set_tests_properties( parser_prec17 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(OP\(STR\(a\), >, STR\(b\)\), <, STR\(c\)\);.*")

add_test(NAME parser_paren1 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/parenthesis1.txt)
set_tests_properties( parser_paren1 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(OP\(STR\(x\), \+, STR\(y\)\), \*, STR\(z\)\);.*")
add_test(NAME parser_paren2 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/parenthesis2.txt)
set_tests_properties( parser_paren2 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), \*, OP\(STR\(y\), +, STR\(z\)\)\);.*")
add_test(NAME parser_paren3 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/parenthesis3.txt)
set_tests_properties( parser_paren3 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(OP\(STR\(x\), \+, STR\(y\)\), <, STR\(z\)\);.*")
add_test(NAME parser_paren4 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/parenthesis4.txt)
set_tests_properties( parser_paren4 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), >, OP\(STR\(y\), \+, STR\(z\)\)\);.*")
add_test(NAME parser_paren5 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/parenthesis5.txt)
set_tests_properties( parser_paren5 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(OP\(STR\(x\), &&, STR\(y\)\), ||, STR\(z\)\);.*")
add_test(NAME parser_paren6 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/parenthesis6.txt)
set_tests_properties( parser_paren6 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), &&, OP\(STR\(y\), ||, STR\(z\)\)\);.*")
add_test(NAME parser_paren7 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/parenthesis7.txt)
set_tests_properties( parser_paren7 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_STR\(x\);.*")
add_test(NAME parser_paren8 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/parenthesis8.txt)
set_tests_properties( parser_paren8 PROPERTIES PASS_REGULAR_EXPRESSION ".*EXP_OP\(STR\(x\), \+, OP\(STR\(y\), \*, OP\(STR\(z\), \+, STR\(w\)\)\)\);.*")

add_test(NAME parser_stmt_var COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/stmt_var.txt)
set_tests_properties( parser_stmt_var PROPERTIES PASS_REGULAR_EXPRESSION ".*VAR\(STR\(x\), NUM\(3\)\);.*")
add_test(NAME parser_stmt_if COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/stmt_if.txt)
set_tests_properties( parser_stmt_if PROPERTIES PASS_REGULAR_EXPRESSION ".*IF\(EXP_OP\(STR\(x\), >, NUM\(0\)\), OP\(STR\(x\), =, OP\(STR\(x\), -, NUM\(1\)\)\);, OP\(STR\(x\), =, OP\(STR\(x\), +, NUM\(1\)\)\);\);.*")
add_test(NAME parser_stmt_while COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/stmt_while.txt)
set_tests_properties( parser_stmt_while PROPERTIES PASS_REGULAR_EXPRESSION ".*LOOP\(OP\(STR\(x\), <, NUM\(10\)\), OP\(STR\(x\), =, OP\(STR\(x\), +, NUM\(1\)\)\);\);.*")
add_test(NAME parser_stmt_for COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/stmt_for.txt)
set_tests_properties( parser_stmt_for PROPERTIES PASS_REGULAR_EXPRESSION ".*VAR\(STR\(i\), NUM\(0\)\); LOOP\(OP\(STR\(i\), <, NUM\(10\)\), CALL\(6, STR\(i\)\); UNARY\(STR\(i\), \+\+\););.*")

add_test(NAME parser_call_print COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/call_print.txt)
set_tests_properties( parser_call_print PROPERTIES PASS_REGULAR_EXPRESSION ".*CALL\(6, STR\(x\)\);.*")
add_test(NAME parser_call_input COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/call_input.txt)
set_tests_properties( parser_call_input PROPERTIES PASS_REGULAR_EXPRESSION ".*CALL\(7, NULL\);.*")
add_test(NAME parser_call_break COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/call_break.txt)
set_tests_properties( parser_call_break PROPERTIES PASS_REGULAR_EXPRESSION ".*CALL\(8, NULL\);.*")

add_test(NAME parser_array1 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/array1.txt)
set_tests_properties( parser_array1 PROPERTIES PASS_REGULAR_EXPRESSION ".*ARR\(STR\(x\), NUM\(3\)\);.*")
add_test(NAME parser_array2 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/array2.txt)
set_tests_properties( parser_array2 PROPERTIES PASS_REGULAR_EXPRESSION ".*ARR\(ARR\(STR\(x\), NUM\(1\)\), STR\(y\)\);.*")
add_test(NAME parser_array3 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/array3.txt)
set_tests_properties( parser_array3 PROPERTIES PASS_REGULAR_EXPRESSION ".*ARR\(ARR\(ARR\(STR\(x\), NUM\(1\)\), NUM\(2\)\), NUM\(3\)\);.*")
add_test(NAME parser_array4 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/array4.txt)
set_tests_properties( parser_array4 PROPERTIES PASS_REGULAR_EXPRESSION ".*ARR_LIT\(NUM\(0\)\);.*")
add_test(NAME parser_array5 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/array5.txt)
set_tests_properties( parser_array5 PROPERTIES PASS_REGULAR_EXPRESSION ".*ARR_LIT\(ARR_LIT\(NUM\(1\), NUM\(2\)\), NUM\(3\)\);.*")
add_test(NAME parser_array6 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/array6.txt)
set_tests_properties( parser_array6 PROPERTIES PASS_REGULAR_EXPRESSION ".*VAR\(ARR\(ARR\(STR\(x\), NUM\(1\)\), NUM\(3\)\), ARR_LIT\(ARR_LIT\(NUM\(1\), NUM\(2\)\), ARR_LIT\(NUM\(4\), NUM\(6\)\)\)\);.*")
add_test(NAME parser_array7 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/array7.txt)
set_tests_properties( parser_array7 PROPERTIES PASS_REGULAR_EXPRESSION ".*VAR\(ARR\(STR\(x\), NULL\), NULL\);.*")

add_test(NAME parser_string1 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/string_simple.txt)
set_tests_properties( parser_string1 PROPERTIES PASS_REGULAR_EXPRESSION ".*ARR_LIT\(NUM\(97\), NUM\(98\), NUM\(99\)\);.*")
add_test(NAME parser_string2 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/string_newline.txt)
set_tests_properties( parser_string2 PROPERTIES PASS_REGULAR_EXPRESSION ".*ARR_LIT\(NUM\(97\), NUM\(10\), NUM\(98\)\);.*")
add_test(NAME parser_string3 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/string_quote.txt)
set_tests_properties( parser_string3 PROPERTIES PASS_REGULAR_EXPRESSION ".*ARR_LIT\(NUM\(97\), NUM\(34\), NUM\(98\)\);.*")
add_test(NAME parser_string4 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/string_backslash.txt)
set_tests_properties( parser_string4 PROPERTIES PASS_REGULAR_EXPRESSION ".*ARR_LIT\(NUM\(97\), NUM\(92\), NUM\(98\)\);.*")
add_test(NAME parser_string5 COMMAND parser ${CMAKE_SOURCE_DIR}/tests/valid/string_tab.txt)
set_tests_properties( parser_string5 PROPERTIES PASS_REGULAR_EXPRESSION ".*ARR_LIT\(NUM\(97\), NUM\(9\), NUM\(98\)\);.*")


# I DON'T CARE HOW STUPID THIS LOOKS. IT TOOK HOURS. DO NOT TOUCH!!!!!!!!
#add_test(NAME post_test_coverage COMMAND  make coverage)


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