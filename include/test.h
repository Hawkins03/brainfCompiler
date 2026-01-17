/** @file test.h
 *  @brief the file providing utility
 * 	testing functions to determine that the code works as intended
 * 
 * eventually I'll add more, but for now it's just test_file
 * @author Hawkins Peterson (Hawkins03)
 * @bug no bugs
 */

#ifndef TEST_H
#define TEST_H


#include "stmt.h"
#include "exp.h"


/** @brief tests to see if the stmt provided by the file matches expectations 
 * 
 * only prints the mismatch or if there's an error, for readability's sake
 * 
 * @param input_file the filename to test
 * @param expected the expected output stmt in string form
 */
int test_file(const char *input_file, const char *expected);

#endif //TEST_H
