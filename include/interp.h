#ifndef BF_H
#define BF_H

/** @file interp.h
 *  @brief Function prototypes for interpreting brainf expressions.
 *
 *  This contains the prototypes for
 *  interpreting brainf code.
 * 
 *  eventually, it'll also be able to interprate the ir into brainf
 *
 *  @author Hawkins Peterson (hawkins03)
 *  @bug No known bugs.
 */

#define BUFF_SIZE 2048

/** @brief interprates a string of brainf code,
 * and takes input and prints output as required
 * 
 * interprates a string of brainf code,
 * and takes input and prints output as required
 * 
 * @param input_buf a buffer of a maximum 2048
 * characters containing a brainf program.
*/
void interp(char *input_buff);

#endif //BF_H
