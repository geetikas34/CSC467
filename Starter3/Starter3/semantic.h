#ifndef _SEMANTIC_H
#define _SEMANTIC_H

#include "ast.h"
#include "symbol.h"

typedef enum {
	ATTRIBUTE /* Read-only, non-constant. */,
	UNIFORM  /* Read-only, constant. These can be assigned to const qualified variables. */,
	RESULT /* Write-only, cannot be assigned anywhere in the scope of an if or else statement. */,
	DEFAULT /* Not a predefined variable */
} type_class;

int semantic_check( node *ast);
type_t check_operator_operand_match(node*, bool);
type_t check_function_semantics(node*);
type_t check_constructor_semantics(node*);
type_t check_var_semantics(node*);
type_t check_decl_semantics(node*);
type_t check_assn_semantics(node* ast);
type_t check_expr_semantics(node* ast);

void print_error(char* msg, int yyline);

bool is_unary_op(int op);
bool is_logical_op(int op);
bool is_arithmetic_type(type_t);

int get_boolean_type(type_t);
int get_int_type(type_t);
int get_float_type(type_t);

type_t get_global_var_type(char* identifier);
type_class get_global_var_class(char* identifier);
int get_type_length(type_t);

#endif
