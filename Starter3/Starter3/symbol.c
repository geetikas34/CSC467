#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ast.h"
#include "symbol.h"

void add_symbol_table_entry(symbol_table_entry* entry, symbol_table* table) {
	entry->next = table->head;
	table->head = entry;
}

void add_global_variables_to_table(symbol_table* table) {

	symbol_table_entry** entry = (symbol_table_entry**) malloc(13 * sizeof(symbol_table_entry*));
	int i;
	for (i=0; i<13; i++) {
		entry[i] = (symbol_table_entry*) malloc(sizeof(symbol_table_entry));
	}
	entry[0]->identifier = strdup("gl_FragColor");
	entry[0]->type = VEC4;
	entry[0]->is_constant = 0;

	add_symbol_table_entry(entry[0], table);
	entry[1]->identifier = strdup("gl_FragDepth");
	entry[1]->type = BOOL;
	entry[1]->is_constant = 0;

	add_symbol_table_entry(entry[1], table);

	entry[2]->identifier = strdup("gl_FragCoord");
	entry[2]->type = VEC4;
	entry[2]->is_constant = 0;

	add_symbol_table_entry(entry[2], table);

	entry[3]->identifier = strdup("gl_TexCoord");
	entry[3]->type = VEC4;
	entry[3]->is_constant = 0;

	add_symbol_table_entry(entry[3], table);

	entry[4]->identifier = strdup("gl_Color");
	entry[4]->type = VEC4;
	entry[4]->is_constant = 0;

	add_symbol_table_entry(entry[4], table);

	entry[5]->identifier = strdup("gl_Secondary");
	entry[5]->type = VEC4;
	entry[5]->is_constant = 0;

	add_symbol_table_entry(entry[5], table);

	entry[6]->identifier = strdup("gl_FogFragCoord");
	entry[6]->type = VEC4;
	entry[6]->is_constant = 0;

	add_symbol_table_entry(entry[6], table);

	entry[7]->identifier = strdup("gl_Light_Half");
	entry[7]->type = VEC4;
	entry[7]->is_constant = 1;

	add_symbol_table_entry(entry[7], table);

	entry[8]->identifier = strdup("gl_Light_Ambient");
	entry[8]->type = VEC4;
	entry[8]->is_constant = 1;

	add_symbol_table_entry(entry[8], table);

	entry[9]->identifier = strdup("gl_Material_Shininess");
	entry[9]->type = VEC4;
	entry[9]->is_constant = 1;

	add_symbol_table_entry(entry[9], table);

	entry[10]->identifier = strdup("env1");
	entry[10]->type = VEC4;
	entry[10]->is_constant = 1;

	add_symbol_table_entry(entry[10], table);

	entry[11]->identifier = strdup("env2");
	entry[11]->type = VEC4;
	entry[11]->is_constant = 1;

	add_symbol_table_entry(entry[11], table);

	entry[12]->identifier = strdup("env3");
	entry[12]->type = VEC4;
	entry[12]->is_constant = 1;

	add_symbol_table_entry(entry[12], table);
}

void delete_table(symbol_table* table) {

	while(table->head) {
		symbol_table_entry* entry = table->head;
		table->head = entry->next;
		free(entry);
	}
	free(table);
	table = NULL;
}

void print_table(symbol_table* table) {

	symbol_table_entry* ptr = table->head;
	int i = 0;
	while(ptr) {
		printf("%d: ID = %s, type = %d, is_const = %d\n", i, ptr->identifier, ptr->type, ptr->is_constant);
		ptr = ptr->next;
		i++;
	}
}

symbol_table_entry* search_symbol_table_entry(symbol_table* table, char* identifier) {
	symbol_table_entry* ptr = table->head;
	while(ptr) {
		if (strcmp(ptr->identifier, identifier) == 0) {
			return ptr;
		}
		ptr = ptr->next;
	}
	return NULL;
}

bool found_duplicate_symbols(symbol_table* table, char* identifier) {
	symbol_table_entry* ptr = table->head;
	symbol_table_entry* prev = NULL;
	symbol_table_entry* temp = NULL;
	int copies = 0;
	while(ptr) {
		if (strcmp(ptr->identifier, identifier) == 0) {
			copies++;
		}
		if(copies >1){
			//rmv entry
			if(prev == NULL){
				temp = ptr;
				ptr = ptr->next;
				free(temp);
				table->head = ptr;
			}
			else{
				prev->next = ptr->next;
				free(ptr);
				ptr = prev->next;
			}
		} else {
			prev = ptr;
			ptr = ptr->next;
		}

	}
	if(copies > 1){
		return true;
	}
	return false;
}

