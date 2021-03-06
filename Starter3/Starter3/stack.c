/*
 * stack.c
 *
 *  Created on: 2015-11-21
 *      Author: saksenag
 */

#include "ast.h"
#include "symbol.h"

void symbol_table_stack_push(symbol_table* table) {
	table->next = stack->head;
	stack->head = table;
}

symbol_table* symbol_table_stack_pop() {
	symbol_table* top = stack->head;
	stack->head = stack->head->next;
	return top;
}


void print_stack() {

	symbol_table* ptr = stack->head;
	int i = 0;
	while(ptr) {
		printf("Stack %d:\n", i);
		print_table(ptr);
		ptr = ptr->next;
		i++;
	}
}

void delete_stack() {
	symbol_table* t = stack->head;
	while(t) {
		delete_table(t);
		t = t->next;
	}
	free(stack);
}
