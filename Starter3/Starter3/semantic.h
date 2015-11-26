#ifndef _SEMANTIC_H
#define _SEMANTIC_H

#include "ast.h"
#include "symbol.h"

typedef enum {
	UNDEFINED_VAR,
	TYPE_MISMATCH,
	LOGICAL_OP,
	ARITHMETIC_OP,
	COMPARISION_OP
} error_code;

int semantic_check( node *ast);
//void semantic_check(node*);
type_t check_operator_operand_match(int, type_t, type_t);
type_t check_function_semantics(node*);
type_t check_constructor_semantics(node*);
type_t check_var_semantics(node*);
type_t check_decl_semantics(node*);
type_t get_global_var_type(char* identifier);
void print_error(char* msg);

bool is_unary_op(int op);
bool is_logical_op(int op);
bool is_arithmetic_type(type_t);

int get_boolean_type(type_t);
int get_int_type(type_t);
int get_float_type(type_t);


#endif
