set(SEM_DIR "${CMAKE_SOURCE_DIR}/tests/sem")

add_test(NAME sem_vars_1 COMMAND semChecker ${SEM_DIR}/p_vars_1.txt)
add_test(NAME sem_vars_2 COMMAND semChecker ${SEM_DIR}/p_vars_2.txt)
add_test(NAME sem_vars_3 COMMAND semChecker ${SEM_DIR}/p_vars_3.txt)
add_test(NAME sem_arrs COMMAND semChecker ${SEM_DIR}/p_arrs.txt)
add_test(NAME sem_branch_1 COMMAND semChecker ${SEM_DIR}/p_branch_1.txt)
add_test(NAME sem_branch_2 COMMAND semChecker ${SEM_DIR}/p_branch_2.txt)
add_test(NAME sem_branch_3 COMMAND semChecker ${SEM_DIR}/p_branch_3.txt)

add_test(NAME sem_fail_immut COMMAND semChecker ${SEM_DIR}/f_immut.txt)
set_tests_properties(sem_fail_immut PROPERTIES WILL_FAIL TRUE FAIL_REGULAR_EXPRESSION "ERR_IMMUT")

add_test(NAME sem_fail_arr_1a COMMAND semChecker ${SEM_DIR}/f_inv_arr_1a.txt)
set_tests_properties(sem_fail_arr_1a PROPERTIES WILL_FAIL TRUE FAIL_REGULAR_EXPRESSION "ERR_INV_ARR")

add_test(NAME sem_fail_arr_1b COMMAND semChecker ${SEM_DIR}/f_inv_arr_1b.txt)
set_tests_properties(sem_fail_arr_1b PROPERTIES WILL_FAIL TRUE FAIL_REGULAR_EXPRESSION "ERR_INV_ARR")

add_test(NAME sem_fail_arr_1c COMMAND semChecker ${SEM_DIR}/f_inv_arr_1c.txt)
set_tests_properties(sem_fail_arr_1c PROPERTIES WILL_FAIL TRUE FAIL_REGULAR_EXPRESSION "ERR_INV_ARR")

add_test(NAME sem_fail_arr_1d COMMAND semChecker ${SEM_DIR}/f_inv_arr_1d.txt)
set_tests_properties(sem_fail_arr_1d PROPERTIES WILL_FAIL TRUE FAIL_REGULAR_EXPRESSION "ERR_INV_ARR")

add_test(NAME sem_fail_arr_2 COMMAND semChecker ${SEM_DIR}/f_inv_arr_2.txt)
set_tests_properties(sem_fail_arr_2 PROPERTIES WILL_FAIL TRUE FAIL_REGULAR_EXPRESSION "ERR_INV_ARR")

add_test(NAME sem_fail_arr_3 COMMAND semChecker ${SEM_DIR}/f_inv_arr_3.txt)
set_tests_properties(sem_fail_arr_3 PROPERTIES WILL_FAIL TRUE FAIL_REGULAR_EXPRESSION "ERR_INV_ARR")

add_test(NAME sem_fail_arr_4 COMMAND semChecker ${SEM_DIR}/f_inv_arr_4.txt)
set_tests_properties(sem_fail_arr_4 PROPERTIES WILL_FAIL TRUE FAIL_REGULAR_EXPRESSION "ERR_INV_ARR")

add_test(NAME sem_fail_arr_5 COMMAND semChecker ${SEM_DIR}/f_inv_arr_4.txt)
set_tests_properties(sem_fail_arr_5 PROPERTIES WILL_FAIL TRUE FAIL_REGULAR_EXPRESSION "ERR_INV_ARR")

add_test(NAME sem_fail_arr_6 COMMAND semChecker ${SEM_DIR}/f_inv_arr_4.txt)
set_tests_properties(sem_fail_arr_6 PROPERTIES WILL_FAIL TRUE FAIL_REGULAR_EXPRESSION "ERR_INV_ARR")

add_test(NAME sem_fail_arr_7 COMMAND semChecker ${SEM_DIR}/f_inv_arr_7.txt)
set_tests_properties(sem_fail_arr_7 PROPERTIES WILL_FAIL TRUE FAIL_REGULAR_EXPRESSION "ERR_INV_ARR")

add_test(NAME sem_fail_arr_8 COMMAND semChecker ${SEM_DIR}/f_inv_arr_8.txt)
set_tests_properties(sem_fail_arr_8 PROPERTIES WILL_FAIL TRUE FAIL_REGULAR_EXPRESSION "ERR_INV_ARR")

add_test(NAME sem_fail_no_var COMMAND semChecker ${SEM_DIR}/f_no_var.txt)
set_tests_properties(sem_fail_no_var PROPERTIES WILL_FAIL TRUE FAIL_REGULAR_EXPRESSION "ERR_NO_VAR")

add_test(NAME sem_fail_redef COMMAND semChecker ${SEM_DIR}/f_redef.txt)
set_tests_properties(sem_fail_redef PROPERTIES WILL_FAIL TRUE FAIL_REGULAR_EXPRESSION "ERR_REDEF")

