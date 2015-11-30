
#ifndef _SYMBOL_H
#define _SYMBOL_H

/* symbol table: linked list of symbol table entries*/
struct symbol_table {
	symbol_table_entry* head;
	symbol_table* next;
	symbol_table* parent;
};

typedef enum {
	ATTRIBUTE /* Read-only, non-constant. */,
	UNIFORM  /* Read-only, constant. These can be assigned to const qualified variables. */,
	RESULT /* Write-only, cannot be assigned anywhere in the scope of an if or else statement. */,
	DEFAULT /* Not a predefined variable */
} type_class;

/* one entry of symbol table */
struct symbol_table_entry {
	char* identifier;
	type_t type;
	int is_constant;
	type_class t_class;
	symbol_table_entry* next;
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

#endif
