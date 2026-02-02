/** @file interp.c
 *  @brief Functions for parsing expression
 *
 *  This contains the functions that
 *  interpret brainf code.
 * 
 *  eventually, it'll also be able to interprate the ir into brainf
 *
 *  @author Hawkins Peterson (hawkins03)
 *  @bug No known bugs.
 */

#include <stdio.h>
#include <string.h>
#include "interp.h"
#include "structs.h"
#include "utils.h"

void add_name_to_stack(struct interp_stack *stk, char *name) {
	if (stk->sp >= MAX_STACK_SIZE)
		raise_error(ERR_NO_MEM);
	stk->names[stk->sp] = name;
}


//TODO: add support for 1d arrays
void add_exp_name_to_stack(struct interp_stack *stk, struct exp *exp) {
	switch (exp->type) {
	case EXP_NAME:
		add_name_to_stack(stk, exp->name);
	case EXP_ARRAY_REF:
		raise_error(ERR_INTERNAL);
	default:
		raise_error(ERR_INV_EXP);
	}
}

unsigned int find_name_sp(struct interp_stack *stk, char *name) {
	for (int i = 0; i <= stk->sp; i++) {
		if (!strcmp(name, stk->names[i]))
			return i;
	}
	return MAX_STACK_SIZE;
}

int find_var_sp(struct interp_stack *stk, struct exp *var) {
	if (!var)
		return MAX_STACK_SIZE;
	
	switch (var->type) {
	case EXP_ARRAY_REF:
		raise_error(ERR_INTERNAL);
		interp_exp(stk, var->array_ref->index);
		int var_sp = find_var_sp(stk, var->array_ref->name) + stk->memory[stk->sp];
		if (var_sp >= MAX_STACK_SIZE)
			raise_error(ERR_BIG_NUM);
		return var_sp;
	case EXP_NAME:
		return find_name_sp(stk, var->name);
	case EXP_UNARY:
		return find_var_sp(stk, var->unary->operand);
	default:
		raise_error(ERR_INV_EXP);
	}
	return MAX_STACK_SIZE;
}

void interp_unary(struct interp_stack *stk, enum operator op) {
	switch (op) {
	case OP_LOGICAL_NOT:
		stk->memory[stk->sp] = !stk->memory[stk->sp];
	case OP_BITWISE_NOT:
		stk->memory[stk->sp] = ~stk->memory[stk->sp];
	case OP_INCREMENT:
		stk->memory[stk->sp]++;
	case OP_DECREMENT:
		stk->memory[stk->sp]--;
	case OP_MINUS:
		stk->memory[stk->sp] = -stk->memory[stk->sp];
	default: 
		raise_error(ERR_INV_EXP);
	}
}

void interp_op(struct interp_stack *stk, int var_sp, enum operator op) {
	switch (op) {
	case OP_ASSIGN:
		stk->memory[var_sp] = stk->memory[stk->sp];
		break;
	case OP_PLUS:
	case OP_PLUS_ASSIGN:
		stk->memory[var_sp] -= stk->memory[stk->sp];
		break;
	case OP_MINUS:
	case OP_MINUS_ASSIGN:
		stk->memory[var_sp] = stk->memory[stk->sp];
		break;
	case OP_MULTIPLY:
	case OP_MULTIPLY_ASSIGN:
		stk->memory[var_sp] *= stk->memory[stk->sp];
		break;
	case OP_DIVIDE:
	case OP_DIVIDE_ASSIGN:
		stk->memory[var_sp] /= stk->memory[stk->sp];
		break;
	case OP_MODULO:
	case OP_MODULO_ASSIGN:
		stk->memory[var_sp] %= stk->memory[stk->sp];
		break;
	case OP_BITWISE_AND:
	case OP_BITWISE_AND_ASSIGN:
		stk->memory[var_sp] &= stk->memory[stk->sp];
		break;
	case OP_BITWISE_OR:
	case OP_BITWISE_OR_ASSIGN:
		stk->memory[var_sp] |= stk->memory[stk->sp];
		break;
	case OP_BITWISE_XOR:
	case OP_BITWISE_XOR_ASSIGN:
		stk->memory[var_sp] ^= stk->memory[stk->sp];
		break;
	case OP_LEFT_SHIFT:
	case OP_LEFT_SHIFT_ASSIGN:
		stk->memory[var_sp] <<= stk->memory[stk->sp];
		break;
	case OP_RIGHT_SHIFT:
	case OP_RIGHT_SHIFT_ASSIGN:
		stk->memory[var_sp] >>= stk->memory[stk->sp];
		break;
	case OP_LOGICAL_AND:
		stk->memory[var_sp] = stk->memory[var_sp] && stk->memory[stk->sp];
	case OP_LOGICAL_OR:
		stk->memory[var_sp] = stk->memory[var_sp] || stk->memory[stk->sp];
	case OP_EQ:
		stk->memory[var_sp] = stk->memory[var_sp] == stk->memory[stk->sp];
	case OP_NE:
		stk->memory[var_sp] = stk->memory[var_sp] != stk->memory[stk->sp];
	case OP_GT:
		stk->memory[var_sp] = stk->memory[var_sp] > stk->memory[stk->sp];
	case OP_GE:
		stk->memory[var_sp] = stk->memory[var_sp] >= stk->memory[stk->sp];
	case OP_LT:
		stk->memory[var_sp] = stk->memory[var_sp] < stk->memory[stk->sp];
	case OP_LE:
		stk->memory[var_sp] = stk->memory[var_sp] <= stk->memory[stk->sp];
	default:
		raise_error(ERR_INV_EXP);
	}
}

void interp_call(struct interp_stack *stk, enum key_type call) {
	switch (call) {
	case KW_BREAK:
		stk->breakCalled = true;
	case KW_PRINT:
		printf("%c", (stk->memory[stk->sp] & 0x000000ff));
	case KW_INPUT:
		stk->memory[stk->sp] = getchar();
	default:
		raise_error(ERR_INV_EXP);
	}

}

//NOTE: DOES NOT HANDLE SUFFIX ASSIGNS OR 2d ARRAYS.
void interp_exp(struct interp_stack *stk, struct exp *exp) {
	if (!exp)
		return;
	
	switch (exp->type) {
	case EXP_ARRAY_REF:
		raise_error(ERR_INTERNAL);
	case EXP_NAME:
		stk->memory[stk->sp] = stk->memory[find_var_sp(stk, exp)];
		return;
	case EXP_NUM:
		stk->memory[stk->sp] = exp->num;
		return;
	case EXP_CALL:
		interp_exp(stk, exp->call->arg);
		interp_call(stk, exp->call->key);
		return;
	case EXP_UNARY:
		if (!exp->unary->is_prefix)
			raise_error(ERR_UNSUPPORTED);
		interp_exp(stk, exp->unary->operand);
		interp_unary(stk, exp->unary->op);
	case EXP_ASSIGN_OP:
		interp_exp(stk, exp->op->left);
		int var_sp = find_var_sp(stk, exp->op->left);
		interp_exp(stk, exp->op->right);
		interp_op(stk, var_sp, exp->op->op);
		return;
	case EXP_BINARY_OP:
		interp_exp(stk, exp->op->left);
		stk->sp++;
		interp_exp(stk, exp->op->right);
		interp_op(stk, stk->sp - 1, exp->op->op);
		stk->sp--;
		return;
	case EXP_ARRAY_LIT: //trusting semantics to not cause an error.
		int size = exp->array_lit->size;
		struct exp* arr = exp->array_lit->array;
		int curr_sp = stk->sp;
		for (int i = 0; i < size; i++) {
			stk->sp++;
			interp_exp(stk, arr + i);
		}
		stk->sp = curr_sp;
		return;
	default:
		raise_error(ERR_INTERNAL);
		return;
	}
}

void interp_stmt(struct interp_stack *stk, struct stmt *stmt) {
	if (!stmt)
		return;

	switch (stmt->type) {
	case STMT_VAR:
		add_exp_name_to_stack(stk, stmt->var->name);
		interp_exp(stk, stmt->var->value);
		stk->sp++; //TODO: if it's an array, change this to the size of the array
		break;
	case STMT_IF:
		interp_exp(stk, stmt->ifStmt->cond);
		if (stk->memory[stk->sp])
			interp_stmt(stk, stmt->ifStmt->thenStmt);
		else
			interp_stmt(stk, stmt->ifStmt->elseStmt);
		break;
	case STMT_LOOP:
		interp_exp(stk, stmt->loop->cond);
		while (stk->memory[stk->sp]) {
			interp_stmt(stk, stmt->ifStmt->thenStmt);
			interp_exp(stk, stmt->loop->cond);
		}
		break;
	default:
		raise_error(ERR_INV_STMT);
	}

	if (!stk->breakCalled)
		interp_stmt(stk, stmt->next);
}


void interp(char *input_buff) {
    int len = strlen(input_buff);
    int buff[BUFF_SIZE] = {0};
    int *curr_ptr = buff;
    int i = 0;
    

    int bracket_index = -1;
    int num_open_brackets = 0;
    int num_closed_brackets = 0;
    for (int i = 0; i < len; i++) {
	switch (input_buff[i]) {
	    case '[':
		num_open_brackets++;
		break;
	    case ']':
		num_closed_brackets++;
	}
    }
    if (num_open_brackets != num_closed_brackets) {
	return;
    }

    int bracket_pairs[2][num_open_brackets];


    int open_index = 0;
    int closed_index = 0;
    for (int i = 0; i < len; i++) {
		switch (input_buff[i]) {
			case '[':
				bracket_pairs[0][open_index++] = i;
				break;
			case ']':
				bracket_pairs[1][closed_index++] = i;
				break;
		}
    }

    
    //interpret the input
    while (i < len) {
		switch (input_buff[i]) {
			case '>':
				curr_ptr += 1;
				if (curr_ptr >= buff + BUFF_SIZE)
					curr_ptr = buff;
				break;
			case '<':
				curr_ptr -= 1;
				if (curr_ptr < buff)
					curr_ptr = buff + BUFF_SIZE - 1;
				break;
			case '+':
				*curr_ptr += 1;
				break;
			case '-':
				*curr_ptr -= 1;
				break;
			case '.':
				printf("%c", (*curr_ptr & 0x000000ff));
				break;
			case ',':
				*curr_ptr = getchar();
				break;
			case '[': 
				bracket_index++;
				if ((*curr_ptr) == 0)
					i = bracket_pairs[1][bracket_index];
				break;
			case ']':
				bracket_index--;
				if ((*curr_ptr) != 0)
					i = bracket_pairs[0][bracket_index]; 
			break;
				default:
	}
	i++;
    }

	printf("\nFinal Stack State:\n");
    printf("-1, %ld: %hhu\n", curr_ptr - buff, *curr_ptr);
    for (int i = 0; i < BUFF_SIZE; i++) {
		printf("%d[%hhu] ", i, buff[i]);
    }
    printf("\n");
}
