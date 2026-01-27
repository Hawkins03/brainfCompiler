set(SEM_DIR "${CMAKE_SOURCE_DIR}/tests/sem")

add_test(NAME sem_vars_1 COMMAND semChecker ${SEM_DIR}/p_vars_1.txt)
add_test(NAME sem_vars_2 COMMAND semChecker ${SEM_DIR}/p_vars_2.txt)
add_test(NAME sem_vars_3 COMMAND semChecker ${SEM_DIR}/p_vars_3.txt)
add_test(NAME sem_arrs COMMAND semChecker ${SEM_DIR}/p_arrs.txt)
add_test(NAME sem_branch_1 COMMAND semChecker ${SEM_DIR}/p_branch_1.txt)
add_test(NAME sem_branch_2 COMMAND semChecker ${SEM_DIR}/p_branch_2.txt)

add_test(NAME sem_fail_else COMMAND semChecker ${SEM_DIR}/f_bad_else.txt)
set_tests_properties(sem_fail_else PROPERTIES WILL_FAIL TRUE FAIL_REGULAR_EXPRESSION "ERR_BAD_ELSE")

add_test(NAME sem_fail_immut COMMAND semChecker ${SEM_DIR}/f_immut.txt)
set_tests_properties(sem_fail_immut PROPERTIES WILL_FAIL TRUE FAIL_REGULAR_EXPRESSION "ERR_IMMUT")

add_test(NAME sem_fail_arr_1 COMMAND semChecker ${SEM_DIR}/f_inv_arr_1.txt)
set_tests_properties(sem_fail_arr_1 PROPERTIES WILL_FAIL TRUE FAIL_REGULAR_EXPRESSION "ERR_INV_ARR")

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

add_test(NAME sem_fail_arr_7 COMMAND semChecker ${SEM_DIR}/f_inv_arr_4.txt)
set_tests_properties(sem_fail_arr_7 PROPERTIES WILL_FAIL TRUE FAIL_REGULAR_EXPRESSION "ERR_INV_ARR")

add_test(NAME sem_fail_esc COMMAND semChecker ${SEM_DIR}/f_inv_esc.txt)
set_tests_properties(sem_fail_esc PROPERTIES WILL_FAIL TRUE FAIL_REGULAR_EXPRESSION "ERR_INV_ESC")

add_test(NAME sem_fail_exp_1 COMMAND semChecker ${SEM_DIR}/f_inv_exp_1.txt)
set_tests_properties(sem_fail_exp_1 PROPERTIES WILL_FAIL TRUE FAIL_REGULAR_EXPRESSION "ERR_INV_EXP")

add_test(NAME sem_fail_exp_2 COMMAND semChecker ${SEM_DIR}/f_inv_exp_2.txt)
set_tests_properties(sem_fail_exp_2 PROPERTIES WILL_FAIL TRUE FAIL_REGULAR_EXPRESSION "ERR_INV_EXP")

add_test(NAME sem_fail_inv_var COMMAND semChecker ${SEM_DIR}/f_inv_var.txt)
set_tests_properties(sem_fail_inv_var PROPERTIES WILL_FAIL TRUE FAIL_REGULAR_EXPRESSION "ERR_INV_VAR")

add_test(NAME sem_fail_no_var COMMAND semChecker ${SEM_DIR}/f_no_var.txt)
set_tests_properties(sem_fail_no_var PROPERTIES WILL_FAIL TRUE FAIL_REGULAR_EXPRESSION "ERR_NO_VAR")

add_test(NAME sem_fail_redef COMMAND semChecker ${SEM_DIR}/f_redef.txt)
set_tests_properties(sem_fail_redef PROPERTIES WILL_FAIL TRUE FAIL_REGULAR_EXPRESSION "ERR_REDEF")

add_test(NAME sem_fail_char COMMAND semChecker ${SEM_DIR}/f_unexp_char.txt)
set_tests_properties(sem_fail_char PROPERTIES WILL_FAIL TRUE FAIL_REGULAR_EXPRESSION "ERR_UNEXP_CHAR")

add_test(NAME sem_fail_quote COMMAND semChecker ${SEM_DIR}/f_unexp_quote.txt)
set_tests_properties(sem_fail_quote PROPERTIES WILL_FAIL TRUE FAIL_REGULAR_EXPRESSION "ERR_UNEXP_QUOTE")

