#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "common.h"
#include "parser.tab.h"
#include "symbol.h"

#define DEBUG_PRINT_TREE 0

node *ast = NULL;
symbol_table_stack *stack = (symbol_table_stack*) malloc(
		sizeof(symbol_table_stack));
symbol_table* curr_table = NULL;
node *ast_allocate(node_kind kind, ...) {
	va_list args;

	// make the node
	node *ast = (node *) malloc(sizeof(node));
	memset(ast, 0, sizeof *ast);
	ast->kind = kind;
	va_start(args, kind);
	ast->type = INVALID;
	ast->current_table = curr_table;
	switch (kind) {

	// ...

	case SCOPE_NODE:
		ast->scope.decls = va_arg(args, node*);
		ast->scope.stats = va_arg(args, node*);
		break;
	case UNARY_EXPRESSION_NODE:
		ast->unary_expr.op = va_arg(args, int);
		ast->unary_expr.right = va_arg(args, node*);
		break;
	case BINARY_EXPRESSION_NODE:
		ast->binary_expr.op = va_arg(args, int);
		ast->binary_expr.left = va_arg(args, node*);
		ast->binary_expr.right = va_arg(args, node*);
		break;
	case INT_NODE:
		ast->type = INT;
		ast->int_literal.val = va_arg(args, int);
		break;
	case FLOAT_NODE:
		ast->type = FLOAT;
		ast->float_literal.val = va_arg(args, double);
		break;
	case BOOL_NODE:
		ast->type = BOOL;
		ast->bool_literal.val = va_arg(args, int);
		break;
	case VAR_NODE:
		ast->variable.id = va_arg(args, char*);
		ast->variable.array_index = va_arg(args, int);
		break;
	case FUNCTION_NODE:
		ast->function.function_name = va_arg(args, int);
		ast->function.args = va_arg(args, node*);
		break;
	case CONSTRUCTOR_NODE:
		ast->type = (type_t) va_arg(args, int);
		ast->constructor.args = va_arg(args, node*);
		break;
	case IF_STATEMENT_NODE:
		ast->if_else.exprs = va_arg(args, node*);
		ast->if_else.stats = va_arg(args, node*);
		ast->if_else.else_stats = va_arg(args, node*);
		break;
	case ASSIGNMENT_NODE:
		ast->assignment.var = va_arg(args, node*);
		ast->assignment.expr = va_arg(args, node*);
		break;
	case NESTED_SCOPE_NODE:
		//	ast->
		break;
	case DECLARATION_NODE:
		ast->declaration.constant = va_arg(args, int);
		ast->type = (type_t) va_arg(args, int);
		ast->declaration.id = va_arg(args, char*);
		ast->declaration.expr = va_arg(args, node*);
		break;
	case DECLARATIONS_NODE:
		ast->declarations.decls = va_arg(args, node*);
		ast->declarations.decl = va_arg(args, node*);
		break;
	case STATEMENTS_NODE:
		ast->statements.stats = va_arg(args, node*);
		ast->statements.stat = va_arg(args, node*);
		break;
	case ARGUMENT_NODE:
		ast->arguments.args = va_arg(args, node*);
		ast->arguments.exprs = va_arg(args, node*);
		break;
		// ...

	default:
		break;
	}

	va_end(args);

	return ast;
}

void ast_free(node *ast) {

}

void ast_print(node * ast) {

	if (ast == NULL) {
		return;
	}

	switch (ast->kind) {
	case SCOPE_NODE: {
		printf("(SCOPE ");
		ast_print(ast->scope.decls);
		ast_print(ast->scope.stats);
		printf(") \n");
		break;
	}
	case UNARY_EXPRESSION_NODE: {
		printf("(UNARY %s %s ", get_type(ast->type),
				get_op(ast->unary_expr.op));
		ast_print(ast->unary_expr.right);
		printf(") \n");
		break;
	}
	case BINARY_EXPRESSION_NODE: {
		printf("(BINARY %s %s ", get_type(ast->type),
				get_op(ast->binary_expr.op));
		ast_print(ast->binary_expr.left);
		ast_print(ast->binary_expr.right);
		printf(") \n");
		break;
	}
	case VAR_NODE: {
		if (ast->variable.array_index != -1) {
			printf("(INDEX %s %s %d)\n", get_type(ast->type), ast->variable.id,
					ast->variable.array_index);
		} else {
			printf("%s ", ast->variable.id);
		}
		break;
	}
	case FUNCTION_NODE: {
		printf("(CALL %s ", get_function_name(ast->function.function_name));
		ast_print(ast->function.args);
		printf(") \n");
		break;
	}
	case CONSTRUCTOR_NODE: {
		printf("(CALL %s ", get_type(ast->type));
		ast_print(ast->constructor.args);
		printf(") \n");
		break;
	}
	case STATEMENTS_NODE: {
		printf("(STATEMENTS ");
		ast_print(ast->statements.stats);
		ast_print(ast->statements.stat);
		printf(") \n");
		break;
	}
	case IF_STATEMENT_NODE: {
		printf("(IF ");
		ast_print(ast->if_else.exprs);
		ast_print(ast->if_else.stats);
		ast_print(ast->if_else.else_stats);
		printf(") \n");
		break;
	}
	case ASSIGNMENT_NODE: {
		printf("(ASSIGN %s ", get_type(ast->type));
		ast_print(ast->assignment.var);
		ast_print(ast->assignment.expr);
		printf(") \n");
		break;
	}
	case ARGUMENT_NODE: {
		ast_print(ast->arguments.args);
		ast_print(ast->arguments.exprs);
		break;
	}
	case DECLARATIONS_NODE: {
		printf("(DECLARATIONS ");
		ast_print(ast->declarations.decls);
		ast_print(ast->declarations.decl);
		printf(") \n");
		break;
	}
	case DECLARATION_NODE: {
		printf("(DECLARATION ");
		printf("%s ", ast->declaration.id);
		printf("%s ", get_type(ast->type));
		ast_print(ast->declaration.expr);
		printf(") \n");
		break;
	}
	case INT_NODE: {
		printf("%d ", ast->int_literal.val);
		break;
	}
	case FLOAT_NODE: {
		printf("%f ", ast->float_literal.val);
		break;
	}
	case BOOL_NODE: {
		printf("%s ", get_bool(ast->bool_literal.val));
		break;
	}
	default: {
		printf("DEFAULT\n");
	}
	}

}

char* get_type(type_t type) {
	char* type_str = NULL;
	switch (type) {
	case INT: {
		type_str = strdup("int");
		break;
	}
	case FLOAT: {
		type_str = strdup("float");
		break;
	}
	case BOOL: {
		type_str = strdup("bool");
		break;
	}
	case IVEC2: {
		type_str = strdup("ivec2");
		break;
	}
	case IVEC3: {
		type_str = strdup("ivec3");
		break;
	}
	case IVEC4: {
		type_str = strdup("ivec4");
		break;
	}
	case VEC2: {
		type_str = strdup("vec2");
		break;
	}
	case VEC3: {
		type_str = strdup("vec3");
		break;
	}
	case VEC4: {
		type_str = strdup("vec4");
		break;
	}
	case BVEC2: {
		type_str = strdup("bvec2");
		break;
	}
	case BVEC3: {
		type_str = strdup("bvec3");
		break;
	}
	case BVEC4: {
		type_str = strdup("bvec4");
		break;
	}
	case ANY: {
		type_str = strdup("Any");
		break;
	}
	default: {
		type_str = strdup("Unknown Type\n");
	}

	}
	return type_str;

}

char* get_op(int op) {

	char* op_str = NULL;
	switch (op) {

	case AND:
		op_str = strdup("&&");
		break;
	case OR:
		op_str = strdup("||");
		break;
	case EQ:
		op_str = strdup("==");
		break;
	case NEQ:
		op_str = strdup("!=");
		break;
	case LEQ:
		op_str = strdup("<=");
		break;
	case GEQ:
		op_str = strdup(">=");
		break;
	case '+':
		op_str = strdup("+");
		break;
	case '-':
		op_str = strdup("-");
		break;
	case '*':
		op_str = strdup("*");
		break;
	case '/':
		op_str = strdup("/");
		break;
	case '^':
		op_str = strdup("^");
		break;
	case '<':
		op_str = strdup("<");
		break;
	case '>':
		op_str = strdup(">");
		break;
	case '!':
		op_str = strdup("!");
		break;
	default:
		op_str = strdup("Unknown operator\n");
	}
	return op_str;
}

char* get_bool(int b) {

	char* b_str = NULL;
	switch (b) {
	case 1:
		b_str = strdup("true");
		break;
	case 0:
		b_str = strdup("false");
		break;
	default:
		b_str = strdup("Unknown bool value\n");
	}
	return b_str;
}

char* get_function_name(int func) {
	char* func_str = NULL;
	switch (func) {
	case 0:
		func_str = strdup("dp3");
		break;
	case 1:
		func_str = strdup("lit");
		break;
	case 2:
		func_str = strdup("rsq");
		break;
	default:
		func_str = strdup("Unknown function name\n");
	}
	return func_str;
}
