#include "semantic.h"
#include "ast.h"
#include "common.h"
#include "parser.tab.h"

#define PRINT_ERROR(x,y) print_error(strdup(x), y)

void print_error(char* error_msg, int yyline) {
	fprintf(errorFile, "SEMANTIC ERROR: %s, LINE %d\n", error_msg, yyline);
	errorOccurred = 1;
}

bool if_else_scope = false;
int semantic_check(node *ast) {

	if (ast == NULL)
		return 0;

	switch (ast->kind) {
	case SCOPE_NODE: {
		semantic_check(ast->scope.decls);
		semantic_check(ast->scope.stats);
		break;
	}
	case UNARY_EXPRESSION_NODE: {
		semantic_check(ast->unary_expr.right);
		ast->type = check_operator_operand_match(ast, true);
		break;
	}
	case BINARY_EXPRESSION_NODE: {
		semantic_check(ast->binary_expr.left);
		semantic_check(ast->binary_expr.right);
		ast->type = check_operator_operand_match(ast, false);
		break;
	}
	case VAR_NODE: {
		ast->type = check_var_semantics(ast);
		break;
	}
	case FUNCTION_NODE: {
		semantic_check(ast->function.args);
		ast->type = check_function_semantics(ast);
		break;
	}
	case CONSTRUCTOR_NODE: {
		semantic_check(ast->constructor.args);
		ast->type = check_constructor_semantics(ast);
		break;
	}
	case IF_STATEMENT_NODE: {
		if_else_scope = true;
		semantic_check(ast->if_else.exprs);
		if (get_boolean_type(ast->if_else.exprs->type) != 1) {
			PRINT_ERROR("If condition does not return bool type", ast->line_num);
		}
		semantic_check(ast->if_else.stats);
		semantic_check(ast->if_else.else_stats);
		if_else_scope = false;
		break;
	}
	case ASSIGNMENT_NODE: {
		semantic_check(ast->assignment.var);
		semantic_check(ast->assignment.expr);
		ast->type = check_assn_semantics(ast);
		break;
	}
	case ARGUMENT_NODE: {
		semantic_check(ast->arguments.args);
		semantic_check(ast->arguments.exprs);
		if (ast->arguments.exprs)
			ast->type = ast->arguments.exprs->type;
		break;
	}
	case DECLARATIONS_NODE: {
		semantic_check(ast->declarations.decls);
		semantic_check(ast->declarations.decl);
		ast->type = ast->declarations.decl->type;
		break;
	}
	case DECLARATION_NODE: {
		semantic_check(ast->declaration.expr);
		type_t rhs_type = check_decl_semantics(ast);
		if (rhs_type == ANY) {
			ast->type = ANY;
		} else if (ast->type != rhs_type) {
			ast->type = ANY;
			PRINT_ERROR("LHS and RHS of declaration type mismatch", ast->line_num);
		}

		break;
	}
	case STATEMENTS_NODE: {
		semantic_check(ast->statements.stats);
		semantic_check(ast->statements.stat);
		ast->type = ast->statements.stat->type;
		break;
	}
	case INT_NODE:
	case FLOAT_NODE:
	case BOOL_NODE:
	case IDENT_NODE:
		break;
	}
	return 0;
}



/*
 * returns true if unary operator
 */
bool is_unary_op(int op) {
	if (op == '-' || op == '!')
		return true;
	else
		return false;
}

/*
 * returns true if logical operator
 */
bool is_logical_op(int op) {
	if (op == '!' || op == OR || op == AND)
		return true;
	return false;

}

/*
 * if boolean type, returns vector length (1 if scalar)
 * else, returns 0
 */
int get_boolean_type(type_t type) {
	if (type >= BOOL && type <= BVEC4)
		return (type - BOOL + 1);
	return 0;
}

/*
 * if int type, returns vector length (1 if scalar)
 * else, returns 0
 */
int get_int_type(type_t type) {
	if (type >= INT && type <= IVEC4)
		return (type - INT + 1);
	return 0;
}

/*
 * if float type, returns vector length (1 if scalar)
 * else, returns 0
 */
int get_float_type(type_t type) {
	if (type >= FLOAT && type <= VEC4)
		return (type - FLOAT + 1);
	return 0;
}

bool is_arithmetic_type(type_t type) {
	if (get_int_type(type) > 0 || get_float_type(type) > 0)
		return true;
	return false;
}

/*
 * returns expression type
 * prints an error statement if operator and operands don't match
 */
type_t check_operator_operand_match(node* ast, bool unary) {

	int op;
	type_t type1, type2;

	// check if unary operator
	if(unary) {
		op = ast->unary_expr.op;
		type1 = ast->unary_expr.right->type;
		type2 = ANY;
		if (is_logical_op(op)) {
			if (get_boolean_type(type1) > 0 || type1 == ANY) {
				return type1;
			} else {
				// operator and operand don't match
				PRINT_ERROR("Unary Operator Operand type mismatch", ast->line_num);
				return ANY;
			}
		} else {
			if ((!get_boolean_type(type1) > 0 || type1 == ANY) && op == (int) '-') {
				return type1;
			} else {
				// operator and operand don't match
				PRINT_ERROR("Unary Operator Operand type mismatch!", ast->line_num);
				return ANY;
			}
		}
	} else {
		op = ast->binary_expr.op;
		type1 = ast->binary_expr.left->type;
		type2 = ast->binary_expr.right->type;
		// binary operator
		if (type1 == ANY || type2 == ANY) {
			return ANY;
		}

		switch (op) {
		case '+':
		case '-':
		case EQ:
		case NEQ:
			if (((get_int_type(type1) > 0) && (get_int_type(type1) == get_int_type(type2)))
					|| ((get_float_type(type1) > 0) && (get_float_type(type1) == get_float_type(type2)))) {
				// same base type and order
				return type1;
			} else {
				PRINT_ERROR("+/- Binary Operator Operand type mismatch!", ast->line_num);
				return ANY;
			}
		case '*':
			if ((get_int_type(type1) > 0 && get_int_type(type2) > 0)) {
				// same base type
				if (get_int_type(type1) > 1) {
					// if type1 is vector, type 2 should be vector of same order or scalar
					if ((get_int_type(type1) == get_int_type(type2)) || (get_int_type(type2) == 1)) {
						return type1;
					} else {
						PRINT_ERROR("Binary Operator Operand type mismatch", ast->line_num);
						return ANY;
					}
				} else {
					return type2;
				}
			} else if ((get_float_type(type1) > 0 && get_float_type(type2) > 0)) {
				// same base type
				if (get_float_type(type1) > 1) {
					// if type1 is vector, type 2 should be vector of same order or scalar
					if ((get_float_type(type1) == get_float_type(type2)) || (get_float_type(type2) == 1)) {
						return type1;
					} else {
						PRINT_ERROR("Binary Operator Operand type mismatch", ast->line_num);
						return ANY;
					}
				} else {
					return type2;
				}
			} else {
				PRINT_ERROR("Binary Operator Operand type mismatch", ast->line_num);
				return ANY;
			}
			break;
		case '/':
		case '^':
		case '<':
		case '>':
		case LEQ:
		case GEQ:
			if ((get_int_type(type1) == 1 && get_int_type(type1) == 1) || (get_float_type(type1) == 1 && get_float_type(type1) == 1)) {
				return type1;
			} else {
				PRINT_ERROR("Binary Operator Operand type mismatch", ast->line_num);
				return ANY;
			}
			break;
		case AND:
		case OR:
			if ((get_boolean_type(type1) > 0) && (get_boolean_type(type1) == get_boolean_type(type2)))
				return type1;
			else {
				PRINT_ERROR("Binary Operator Operand type mismatch", ast->line_num);
				return ANY;
			}
			break;
		default:
			PRINT_ERROR("Not any of the above binary operators.", ast->line_num);
			return ANY;
		}
	}
	// control shouldn't reach here
	return ANY;
}

/*
 * Check if function name corresponds with its arguments
 */
type_t check_function_semantics(node* ast) {
	node* ptr = ast->function.args;
	int num_args = 0;
	type_t arg1_t, arg2_t;
	int function_name = ast->function.function_name;

	while (ptr) {
		num_args++;
		if (num_args == 1) {
			arg1_t = ptr->type;
		} else if (num_args == 2) {
			arg2_t = ptr->type;
		} else {
			PRINT_ERROR("More than 2 args in a function", ast->line_num);
			return ANY;
		}
		ptr = ptr->arguments.args;
	}

	if (num_args == 0) {
		PRINT_ERROR("Function has 0 arguments", ast->line_num);
		return ANY;
	}
	if (arg1_t == ANY || ((num_args == 2) && (arg2_t == ANY))) {
		return ANY;
	}

	switch (function_name) {
	case 0: //DP3
		if (num_args == 2) {
			if ((arg1_t == VEC3 && arg2_t == VEC3) || (arg1_t == VEC4 && arg2_t == VEC4)) {
				return FLOAT;
			} else if ((arg1_t == IVEC3 && arg2_t == IVEC3) || (arg1_t == IVEC4 && arg2_t == IVEC4)) {
				return INT;
			} else {
				PRINT_ERROR("DP3 argument type mismatch", ast->line_num);
				return ANY;
			}
		} else {
			PRINT_ERROR("DP3 expects two args", ast->line_num);
			return ANY;
		}
		break;
	case 1: //LIT
		if (num_args == 1) {
			if (arg1_t == VEC4) {
				return VEC4;
			} else {
				PRINT_ERROR("LIT expects arg of VEC4", ast->line_num);
				return ANY;
			}
		} else {
			PRINT_ERROR("LIT expects one arg", ast->line_num);
			return ANY;
		}
	case 2: //RSQ
		if (num_args == 1) {
			if (arg1_t == INT || arg1_t == FLOAT)
				return FLOAT;
			else {
				PRINT_ERROR("RSQ invalid argument type", ast->line_num);
				return ANY;
			}
		} else {
			PRINT_ERROR("RSQ expects one arg", ast->line_num);
			return ANY;
		}
	default:
		PRINT_ERROR("Function name not valid. Should not come here.", ast->line_num);
		return ANY;
	}

	return ANY;
}

type_t check_constructor_semantics(node* ast) {
	node* ptr = ast->constructor.args;
	int num_args = 0;
	type_t arg_t;

	while (ptr) {
		num_args++;
		if (ptr->type == ANY) {
			return ANY;
		}
		if (num_args == 1) {
			arg_t = ptr->type;
		} else if (num_args > 4) {
			PRINT_ERROR("Constructor has more than 4 arguments", ast->line_num);
			return ANY;
		} else if (num_args > 1 && (arg_t != ptr->type)) {
			PRINT_ERROR("Constructor arg types don't match", ast->line_num);
			return ANY;
		}
		ptr = ptr->arguments.args;
	}

	if (num_args == 0) {
		PRINT_ERROR("Constructor has 0 arguments", ast->line_num);
		return ANY;
	} else if (num_args != get_type_length(ast->type)) {
		PRINT_ERROR("Constructor number of arguments do not match type", ast->line_num);
		return ANY;
	}

	return ast->type;

}

type_t check_var_semantics(node* ast) {
	//Check if this id exists in symbol table
	char* id = ast->variable.id;
	symbol_table * t;
	symbol_table_entry* entry;
	for (t = ast->current_table; t != NULL; t = t->parent) {
		entry = search_symbol_table_entry(t, id);
		if (entry)
			break;
	}
	if (!entry) {
		PRINT_ERROR("Variable not declared", ast->line_num);
		return ANY;
	} else {
		int array_index = ast->variable.array_index;
		if (array_index == -1) {
			// ID
			return entry->type;
		} else {
			int blen = get_boolean_type(entry->type);
			int ilen = get_int_type(entry->type);
			int flen = get_float_type(entry->type);
			if (blen != 0) {
				if ((array_index < blen) && (array_index >= 0)) {
					return BOOL;
				} else {
					PRINT_ERROR("Array index out of bounds", ast->line_num);
				}
			} else if (ilen != 0) {
				if ((array_index < ilen) && (array_index >= 0)) {
					return INT;
				} else {
					PRINT_ERROR("Array index out of bounds", ast->line_num);
				}
			} else if (flen != 0) {
				if ((array_index < flen) && (array_index >= 0)) {
					return FLOAT;
				} else {
					PRINT_ERROR("Array index out of bounds", ast->line_num);
				}
			}

		}

	}
	return ANY;
}

type_t check_decl_semantics(node* ast) {
	//loop through the st and check for duplicate ids
	char* id = ast->declaration.id;
	if (found_duplicate_symbols(ast->current_table, id)) {
		PRINT_ERROR("Multiple declarations", ast->line_num);
		return ANY;
	}

	if (!ast->declaration.expr) {
		return ast->type;
	}
	node_kind rhs_kind = ast->declaration.expr->kind;
	type_t rhs_type = ast->declaration.expr->type;

	// check if const var initialised with literal or global var
	if (ast->declaration.constant) {

		if (rhs_kind == VAR_NODE) {
			type_t global_type = get_global_var_type(ast->declaration.expr->variable.id);
			type_class global_class = get_global_var_class(ast->declaration.expr->variable.id);
			if (global_type != INVALID && global_class == UNIFORM) {
				rhs_type = global_type;
			} else {
				PRINT_ERROR("RHS of const declaration is not a uniform var or literal", ast->line_num);
				rhs_type = ANY;
			}
		} else if (rhs_type != INT && rhs_type != FLOAT && rhs_type != BOOL && rhs_kind != CONSTRUCTOR_NODE) {
			PRINT_ERROR("RHS of const declaration is not a uniform var or literal", ast->line_num);
			rhs_type = ANY;
		}
	}

	return rhs_type;
}

type_t check_assn_semantics(node* ast) {

	char* id = ast->assignment.var->variable.id;

	type_t lhs_global_type = get_global_var_type(id);
	type_class lhs_global_class = get_global_var_class(id);

	// check if LHS is a global variable
	if(lhs_global_type != INVALID) {
		// if it is, check if it is write-only, if not it is an error
		if (lhs_global_class != RESULT) {
			PRINT_ERROR("LHS of assignment is a read-only global variable", ast->line_num);
			return ANY;
		} else {
			// cannot assign result in if_else scope
			if (if_else_scope) {
				PRINT_ERROR("Result global variable cannot be written in an if-else scope", ast->line_num);
			}
		}
	}

	symbol_table * t;
	symbol_table_entry* entry;
	for (t = ast->current_table; t != NULL; t = t->parent) {
		entry = search_symbol_table_entry(t, id);
		if (entry)
			break;
	}
	if (entry) {
		if (entry->is_constant) {
			PRINT_ERROR("const variable cannot be redefined", ast->line_num);
			return ANY;
		}
		if(ast->assignment.expr->kind == VAR_NODE) {
			char* rhs_id = ast->assignment.expr->variable.id;
			type_t rhs_global_type = get_global_var_type(rhs_id);
			type_class rhs_global_class = get_global_var_class(rhs_id);
			// if RHS is a global variable check if it is of type ATTRIBUTE, otherwise error
			if (rhs_global_type != INVALID && rhs_global_class == RESULT) {
				PRINT_ERROR("RHS of assignment is a Result type global variable", ast->line_num);
				return ANY;
			}
		}
		if (ast->assignment.var->type == ast->assignment.expr->type || ast->assignment.var->type == ANY || ast->assignment.expr->type == ANY) {
			return ast->assignment.var->type;
		} else {
			PRINT_ERROR("Assignment type mismatch", ast->line_num);
			return ANY;
		}
	}

	return ast->type;
}

type_t get_global_var_type(char* identifier) {
	if ((!strcmp(identifier, "gl_FragColor") || !strcmp(identifier, "gl_FragCoord") || !strcmp(identifier, "gl_TexCoord") || !strcmp(identifier, "gl_Color")
			|| !strcmp(identifier, "gl_Secondary") || !strcmp(identifier, "gl_FogFragCoord") || !strcmp(identifier, "gl_Light_Half")
			|| !strcmp(identifier, "gl_Light_Ambient") || !strcmp(identifier, "gl_Material_Shininess") || !strcmp(identifier, "env1")
			|| !strcmp(identifier, "env2") || !strcmp(identifier, "env3")))
		return VEC4;
	else if (!strcmp(identifier, "gl_FragDepth"))
		return BOOL;
	else
		return INVALID;
}

type_class get_global_var_class(char* identifier) {
	if (!strcmp(identifier, "gl_FragColor") || !strcmp(identifier, "gl_FragDepth"))
		return RESULT;
	else if (!strcmp(identifier, "gl_TexCoord") || !strcmp(identifier, "gl_Color") || !strcmp(identifier, "gl_Secondary")
			|| !strcmp(identifier, "gl_FogFragCoord") || !strcmp(identifier, "gl_FragCoord"))
		return ATTRIBUTE;
	else if (!strcmp(identifier, "gl_Light_Half") || !strcmp(identifier, "gl_Light_Ambient") || !strcmp(identifier, "gl_Material_Shininess")
			|| !strcmp(identifier, "env1") || !strcmp(identifier, "env2") || !strcmp(identifier, "env3"))
		return UNIFORM;
	else
		return DEFAULT;
}

int get_type_length(type_t t) {
	int len;
	switch (t) {
	case INT:
	case IVEC2:
	case IVEC3:
	case IVEC4:
		len = (t - INT) + 1;
		break;
	case FLOAT:
	case VEC2:
	case VEC3:
	case VEC4:
		len = (t - FLOAT) + 1;
		break;
	case BOOL:
	case BVEC2:
	case BVEC3:
	case BVEC4:
		len = (t - BOOL) + 1;
		break;
	case ANY:
	case INVALID:
		len = -1;
		break;
	}
	return len;
}
