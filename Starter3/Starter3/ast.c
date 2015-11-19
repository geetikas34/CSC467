#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>

#include "ast.h"
#include "common.h"
#include "parser.tab.h"

#define DEBUG_PRINT_TREE 0

node *ast = NULL;

node *ast_allocate(node_kind kind, ...) {
  va_list args;

  // make the node
  node *ast = (node *) malloc(sizeof(node));
  memset(ast, 0, sizeof *ast);
  ast->kind = kind;

  va_start(args, kind); 

  switch(kind) {
  
  // ...

  case SCOPE_NODE:
	  ast->scope.declarations
	    break;
/*  case EXPRESSION_NODE:
	    break;*/
  case UNARY_EXPRESION_NODE:
	    break;
  case BINARY_EXPRESSION_NODE:
	    break;
  case INT_NODE:
	    break;
  case FLOAT_NODE:
	    break;
  case IDENT_NODE:
	    break;
  case VAR_NODE:
	    break;
  case FUNCTION_NODE:
	    break;
  case CONSTRUCTOR_NODE:
	    break;
  case STATEMENT_NODE:
	    break;
  case IF_STATEMENT_NODE:
	    break;
  case ASSIGNMENT_NODE:
	    break;
  case NESTED_SCOPE_NODE:
	    break;
  case DECLARATION_NODE:
	    break;
  case BINARY_EXPRESSION_NODE:
    ast->binary_expr.op = va_arg(args, int);
    ast->binary_expr.left = va_arg(args, node *);
    ast->binary_expr.right = va_arg(args, node *);
    break;

  // ...

  default: break;
  }

  va_end(args);

  return ast;
}

void ast_free(node *ast) {

}

void ast_print(node * ast) {

}
