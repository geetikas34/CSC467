#ifndef AST_H_
#define AST_H_


#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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

	SCOPE_NODE = (1 << 0),

	EXPRESSION_NODE = (1 << 2),
	UNARY_EXPRESSION_NODE = (1 << 2) | (1 << 3),
	BINARY_EXPRESSION_NODE = (1 << 2) | (1 << 4),
	INT_NODE = (1 << 2) | (1 << 5),
	FLOAT_NODE = (1 << 2) | (1 << 6),
	BOOL_NODE = (1 << 2) | (1 << 16),
	IDENT_NODE = (1 << 2) | (1 << 7),
	VAR_NODE = (1 << 2) | (1 << 8),
	FUNCTION_NODE = (1 << 2) | (1 << 9),
	CONSTRUCTOR_NODE = (1 << 2) | (1 << 10),
	STATEMENT_NODE = (1 << 1),
	IF_STATEMENT_NODE = (1 << 1) | (1 << 11),
	ASSIGNMENT_NODE = (1 << 1) | (1 << 13),
	NESTED_SCOPE_NODE = (1 << 1) | (1 << 14),
	ARGUMENT_NODE = (1 << 1) | (1 << 17),
	DECLARATION_NODE = (1 << 15),
	DECLARATIONS_NODE = (1 << 1) | (1 << 18)
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
void ast_print(node * ast);

char* get_op(int op);
char* get_type(type_t type);
char* get_bool(int b);
char* get_function_name(int func);

/* one entry of symbol table */
struct symbol_table_entry {
	char* identifier;
	type_t type;
	int is_constant;
	symbol_table_entry* next;
};

/* symbol table: linked list of symbol table entries*/
struct symbol_table {
	symbol_table_entry* head;
	symbol_table* next;
	symbol_table* parent;
};

void add_symbol_table_entry(symbol_table_entry* entry, symbol_table* table);
symbol_table_entry* search_symbol_table_entry(symbol_table* table, char* identifier);
bool found_duplicate_symbols(symbol_table* table, char* identifier);
void add_global_variables_to_table(symbol_table* table);
//void delete_table (symbol_table* table);
void print_table(symbol_table* table);

/* symbol table stack: linked list implementation of symbol table stack */
struct symbol_table_stack {
	symbol_table* head;
};

void symbol_table_stack_push(symbol_table* table);
symbol_table* symbol_table_stack_pop();
void print_stack();

#endif /* AST_H_ */
