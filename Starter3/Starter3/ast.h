#ifndef AST_H_
#define AST_H_


#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
enum {
  DP3 = 0,
  LIT = 1,
  RSQ = 2
};
// Dummy node just so everything compiles, create your own node/nodes
//
// The code provided below is an example ONLY. You can use/modify it,
// but do not assume that it is correct or complete.
//
// There are many ways of making AST nodes. The approach below is an example
// of a descriminated union. If you choose to use C++, then I suggest looking
// into inheritance.

// forward declare
struct node_;
struct symbol_table_entry;
struct symbol_table;
struct symbol_table_stack;

typedef struct node_ node;
typedef struct symbol_table_entry symbol_table_entry;
typedef struct symbol_table symbol_table;
typedef struct symbol_table_stack symbol_table_stack;

extern node *ast;
extern symbol_table_stack *stack;
extern symbol_table* curr_table;
typedef enum {
	UNKNOWN = 0,
	SCOPE_NODE = 1,
	EXPRESSION_NODE = 2,
	UNARY_EXPRESSION_NODE = 3,
	BINARY_EXPRESSION_NODE = 4,
	INT_NODE = 5,
	FLOAT_NODE = 6,
	BOOL_NODE = 7,
	IDENT_NODE = 8,
	VAR_NODE = 9,
	FUNCTION_NODE = 10,
	CONSTRUCTOR_NODE = 11,
	STATEMENT_NODE = 12,
	IF_STATEMENT_NODE = 13,
	ASSIGNMENT_NODE = 14,
	NESTED_SCOPE_NODE = 15,
	ARGUMENT_NODE = 16,
	DECLARATION_NODE = 17,
	DECLARATIONS_NODE = 18,
	STATEMENTS_NODE = 19
} node_kind;

typedef enum {
	INT = 0,
	IVEC2 = 1,
	IVEC3 = 2,
	IVEC4 = 3,
	FLOAT = 4,
	VEC2 = 5,
	VEC3 = 6,
	VEC4 = 7,
	BOOL = 8,
	BVEC2 = 9,
	BVEC3 = 10,
	BVEC4 = 11,
	ANY = 12,
	INVALID = -1
} type_t;

struct node_ {

	// an example of tagging each node with a type
	node_kind kind;
	type_t type;
	symbol_table *current_table;
	int line_num;

	union {

		struct {
			node *decls;
			node *stats;
		} scope;

		struct {
			int op;
			node *right;
		} unary_expr;

		struct {
			int op;
			node *left;
			node *right;
		} binary_expr;

		struct {
			int val;
		} int_literal;

		struct {
			float val;
		} float_literal;

		struct {
			bool val;
		} bool_literal;

		struct {
			char* id;
			int array_index;
		} variable;

		struct {
			int function_name;
			node* args;
		} function;

		struct {
			node* args;
		} constructor;

		struct {
			node *stat;
			node *stats;
		} statements;

		struct {
			node *exprs;
			node *stats;
			node *else_stats;
		} if_else;

		struct {
			node *var;
			node *expr;
		} assignment;

		struct {
			node* args;
			node* exprs;
		} arguments;

		struct {
			int constant;
			char* id;
			node* expr;
		} declaration;

		struct {
			node* decls;
			node* decl;
		} declarations;

	};
};

node *ast_allocate(node_kind type, ...);
void ast_free(node *ast);
void ast_free_helper(node *ast);
void ast_print(node * ast);

char* get_op(int op);
char* get_type(type_t type);
char* get_bool(int b);
char* get_function_name(int func);

#endif /* AST_H_ */
