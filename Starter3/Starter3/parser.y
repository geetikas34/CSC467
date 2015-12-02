%{
/***********************************************************************
 * --YOUR GROUP INFO SHOULD GO HERE--
 * 
 *   Interface to the parser module for CSC467 course project.
 * 
 *   Phase 2: Implement context free grammar for source language, and
 *            parse tracing functionality.
 *   Phase 3: Construct the AST for the source language program.
 ***********************************************************************/

/***********************************************************************
 *  C Definitions and external declarations for this module.
 *
 *  Phase 3: Include ast.h if needed, and declarations for other global or
 *           external vars, functions etc. as needed.
 ***********************************************************************/

#include <string.h>

#include "common.h"
#include "ast.h"
#include "symbol.h"
#include "semantic.h"

#define YYERROR_VERBOSE
#define yTRACE(x)    { if (traceParser) fprintf(traceFile, "%s\n", x); }

void yyerror(char* s);    /* what to do in case of error            */
int yylex();              /* procedure for calling lexical analyzer */
extern int yyline;        /* variable holding current line number   */


%}

/***********************************************************************
 *  Yacc/Bison declarations.
 *  Phase 2:
 *    1. Add precedence declarations for operators (after %start declaration)
 *    2. If necessary, add %type declarations for some nonterminals
 *  Phase 3:
 *    1. Add fields to the union below to facilitate the construction of the
 *       AST (the two existing fields allow the lexical analyzer to pass back
 *       semantic info, so they shouldn't be touched).
 *    2. Add <type> modifiers to appropriate %token declarations (using the
 *       fields of the union) so that semantic information can by passed back
 *       by the scanner.
 *    3. Make the %type declarations for the language non-terminals, utilizing
 *       the fields of the union as well.
 ***********************************************************************/

%{
#define YYDEBUG 1
%}

// defines the yyval union
%union {
  int as_int;
  int as_vec;
  float as_float;
  char *as_str;
  int as_func;
  node *as_ast;
  bool as_bool;
}

%token          FLOAT_T
%token          INT_T
%token          BOOL_T
%token          CONST
%token          FALSE_C TRUE_C
%token          IF ELSE
%token          AND OR NEQ EQ LEQ GEQ

// links specific values of tokens to yyval
%token <as_vec>   VEC_T
%token <as_vec>   BVEC_T
%token <as_vec>   IVEC_T
%token <as_float> FLOAT_C
%token <as_int>   INT_C
%token <as_str>   ID
%token <as_func> FUNC

// operator precdence
%left     OR                        // 7
%left     AND                       // 6
%left     EQ NEQ '<' LEQ '>' GEQ    // 5
%left     '+' '-'                   // 4
%left     '*' '/'                   // 3
%right    '^'                       // 2
%right    '!' UMINUS                // 1
%left     '(' '['                   // 0

// resolve dangling else shift/reduce conflict with associativity
%left     WITHOUT_ELSE
%left     WITH_ELSE

// type declarations
// TODO: fill this out
%type <as_ast> program scope declarations declaration statements statement expression variable arguments arguments_opt
%type <as_int> type 
%type <as_bool> TRUE_C FALSE_C
// expect one shift/reduce conflict, where Bison chooses to shift
// the ELSE.
%expect 1

%start    program

%%

/***********************************************************************
 *  Yacc/Bison rules
 *  Phase 2:
 *    1. Replace grammar found here with something reflecting the source
 *       language grammar
 *    2. Implement the trace parser option of the compiler
 ***********************************************************************/
program
  : scope 
      { yTRACE("program -> scope\n");
      	ast = $1;
      	// stack->head = NULL; 
      	semantic_check(ast);
      } 
  ;

scope
  : '{' 
    { 
    	symbol_table* new_table = (symbol_table*) malloc(sizeof(symbol_table));
    	new_table->head = NULL;
  		if (stack->head == NULL) {
  			add_global_variables_to_table(new_table);
  			new_table->parent = NULL;
  		}
  		else {
  			new_table->parent = curr_table;
  		}
  		curr_table = new_table;
  		symbol_table_stack_push(curr_table);	
  	}
   	declarations statements '}' { 
   		yTRACE("scope -> { declarations statements }\n");
        //print_stack();
        if(curr_table->parent)
        	curr_table = curr_table->parent;
        $$ = ast_allocate(SCOPE_NODE, $3, $4, yyline);
        symbol_table* table = symbol_table_stack_pop();
        if (stack->head == NULL) {
        	free(stack);
        }
    }
  ;

declarations
  : declarations declaration
      { yTRACE("declarations -> declarations declaration\n");
      	$$ = ast_allocate(DECLARATIONS_NODE, $1, $2, yyline);
      }
  | 
      { yTRACE("declarations -> \n");
      	$$ = NULL;
      }
  ;

statements
  : statements statement
      { yTRACE("statements -> statements statement\n");
      	$$ = ast_allocate(STATEMENTS_NODE, $1, $2, yyline);
      }
  | 
      { yTRACE("statements -> \n");
      	$$ = NULL; }
  ;

declaration
  : type ID ';' 
      { yTRACE("declaration -> type ID ;\n");
        $$ = ast_allocate(DECLARATION_NODE, 0, $1, $2, NULL, yyline);
        symbol_table_entry* entry = (symbol_table_entry*) malloc(sizeof(symbol_table_entry));
        entry->identifier = strdup($2);
        entry->type = $1;
        entry->is_constant = 0;
        add_symbol_table_entry(entry, stack->head);
      }
  | type ID '=' expression ';'
      { yTRACE("declaration -> type ID = expression ;\n");
        $$ = ast_allocate(DECLARATION_NODE, 0, $1, $2, $4, yyline);
        symbol_table_entry* entry = (symbol_table_entry*) malloc(sizeof(symbol_table_entry));
        entry->identifier = strdup($2);
        entry->type = $1;
        entry->is_constant = 0;
        add_symbol_table_entry(entry, stack->head);
      }
  | CONST type ID '=' expression ';'
      { yTRACE("declaration -> CONST type ID = expression ;\n");
        $$ = ast_allocate(DECLARATION_NODE, 1, $2, $3, $5, yyline);
        symbol_table_entry* entry = (symbol_table_entry*) malloc(sizeof(symbol_table_entry));
        entry->identifier = strdup($3);
        entry->type = $2;
        entry->is_constant = 1;
        add_symbol_table_entry(entry, stack->head);
      }
  ;

statement
  : variable '=' expression ';'
      { yTRACE("statement -> variable = expression ;\n");
        $$ = ast_allocate(ASSIGNMENT_NODE, $1, $3, yyline);
      }
  | IF '(' expression ')' statement ELSE statement %prec WITH_ELSE
      { yTRACE("statement -> IF ( expression ) statement ELSE statement \n");
        $$ = ast_allocate(IF_STATEMENT_NODE, $3, $5, $7, yyline);
      }
  | IF '(' expression ')' statement %prec WITHOUT_ELSE
      { yTRACE("statement -> IF ( expression ) statement \n");
        $$ = ast_allocate(IF_STATEMENT_NODE, $3, $5, NULL, yyline);
      }
  | scope 
      { yTRACE("statement -> scope \n");
      	$$ = $1; }
  | ';'
      { yTRACE("statement -> ; \n");
      	$$ = NULL;
      }
  ;

type
  : INT_T
      { yTRACE("type -> INT_T \n");
      	$$ = INT; }
  | IVEC_T
      { yTRACE("type -> IVEC_T \n");
      	switch ($1) {
      		case 2: $$ = IVEC2;
      		break;
      		case 3: $$ = IVEC3;
      		break;
      		case 4: $$ = IVEC4;
      		break;
      	}
      }
  | BOOL_T
      { yTRACE("type -> BOOL_T \n");
      	$$ = BOOL;
      }
  | BVEC_T
      { yTRACE("type -> BVEC_T \n");
      	switch ($1) {
      		case 2: $$ = BVEC2;
      		break;
      		case 3: $$ = BVEC3;
      		break;
      		case 4: $$ = BVEC4;
      		break;
      	}
      }
  | FLOAT_T
      { yTRACE("type -> FLOAT_T \n");
      	$$ = FLOAT;
      }
  | VEC_T
      { yTRACE("type -> VEC_T \n");
	      switch ($1) {
	      		case 2: $$ = VEC2;
	      		break;
	      		case 3: $$ = VEC3;
	      		break;
	      		case 4: $$ = VEC4;
      			break;
      	}
      }
  ;

expression

  /* function-like operators */
  : type '(' arguments_opt ')' %prec '('
      { yTRACE("expression -> type ( arguments_opt ) \n");
        $$ = ast_allocate(CONSTRUCTOR_NODE, $1, $3, yyline);
      }
  | FUNC '(' arguments_opt ')' %prec '('
      { yTRACE("expression -> FUNC ( arguments_opt ) \n");
      	$$ = ast_allocate(FUNCTION_NODE, $1, $3, yyline);
      }

  /* unary operators */
  | '-' expression %prec UMINUS
      { yTRACE("expression -> - expression \n");
        $$ = ast_allocate(UNARY_EXPRESSION_NODE, '-', $2, yyline);
      }
  | '!' expression %prec '!'
      { yTRACE("expression -> ! expression \n");
       $$ = ast_allocate(UNARY_EXPRESSION_NODE, '!', $2, yyline);
      }

  /* binary operators */
  | expression AND expression %prec AND
      { yTRACE("expression -> expression AND expression \n");
      	$$ = ast_allocate(BINARY_EXPRESSION_NODE, AND, $1, $3, yyline);
      }
  | expression OR expression %prec OR
      { yTRACE("expression -> expression OR expression \n");
      	$$ = ast_allocate(BINARY_EXPRESSION_NODE, OR, $1, $3, yyline);
      }
  | expression EQ expression %prec EQ
      { yTRACE("expression -> expression EQ expression \n"); 
      	$$ = ast_allocate(BINARY_EXPRESSION_NODE, EQ, $1, $3, yyline);
      }
  | expression NEQ expression %prec NEQ
      { yTRACE("expression -> expression NEQ expression \n");
        $$ = ast_allocate(BINARY_EXPRESSION_NODE, NEQ, $1, $3, yyline);
      }
  | expression '<' expression %prec '<'
      { yTRACE("expression -> expression < expression \n");
      	$$ = ast_allocate(BINARY_EXPRESSION_NODE, EQ, $1, $3, yyline);
      }
  | expression LEQ expression %prec LEQ
      { yTRACE("expression -> expression LEQ expression \n");
     	$$ = ast_allocate(BINARY_EXPRESSION_NODE, LEQ, $1, $3, yyline);
      }
  | expression '>' expression %prec '>'
      { yTRACE("expression -> expression > expression \n");
      	$$ = ast_allocate(BINARY_EXPRESSION_NODE, '>', $1, $3, yyline);
      }
  | expression GEQ expression %prec GEQ
      { yTRACE("expression -> expression GEQ expression \n");
      	$$ = ast_allocate(BINARY_EXPRESSION_NODE, GEQ, $1, $3, yyline);	
      }
  | expression '+' expression %prec '+'
      { yTRACE("expression -> expression + expression \n");
     	$$ = ast_allocate(BINARY_EXPRESSION_NODE, '+', $1, $3, yyline);
      }
  | expression '-' expression %prec '-'
      { yTRACE("expression -> expression - expression \n");
      	$$ = ast_allocate(BINARY_EXPRESSION_NODE, '-', $1, $3, yyline);
      }
  | expression '*' expression %prec '*'
      { yTRACE("expression -> expression * expression \n");
        $$ = ast_allocate(BINARY_EXPRESSION_NODE, '*', $1, $3, yyline);
      }
  | expression '/' expression %prec '/'
      { yTRACE("expression -> expression / expression \n");
        $$ = ast_allocate(BINARY_EXPRESSION_NODE, '/', $1, $3, yyline);
      }
  | expression '^' expression %prec '^'
      { yTRACE("expression -> expression ^ expression \n");
        $$ = ast_allocate(BINARY_EXPRESSION_NODE, '^', $1, $3, yyline);
      }
 
  /* literals */
  | TRUE_C
      { yTRACE("expression -> TRUE_C \n");
      	$$ = ast_allocate(BOOL_NODE, true, yyline);
      }
  | FALSE_C
      { yTRACE("expression -> FALSE_C \n");
      	$$ = ast_allocate(BOOL_NODE, false, yyline);
      }
  | INT_C
      { yTRACE("expression -> INT_C \n");
      	$$ = ast_allocate(INT_NODE, $1, yyline);
      }
  | FLOAT_C
      { yTRACE("expression -> FLOAT_C \n");
      	$$ = ast_allocate(FLOAT_NODE, $1, yyline);
      }

  /* misc */
  | '(' expression ')'
      { yTRACE("expression -> ( expression ) \n"); 
      	$$ = $2;
      }
  | variable 
    { yTRACE("expression -> variable \n");
      $$ = $1;
    }
  ;

variable
  : ID
      { yTRACE("variable -> ID \n");
      	$$ = ast_allocate(VAR_NODE, $1, -1, yyline);
      }
  | ID '[' INT_C ']' %prec '['
      { yTRACE("variable -> ID [ INT_C ] \n");
      	$$ = ast_allocate(VAR_NODE, $1, $3, yyline); 
      }
  ;

arguments
  : arguments ',' expression
      { yTRACE("arguments -> arguments , expression \n");
      	$$ = ast_allocate(ARGUMENT_NODE, $1, $3, yyline);
      }
  | expression
      { yTRACE("arguments -> expression \n");
      	$$ = ast_allocate(ARGUMENT_NODE, NULL, $1, yyline);
      }
  ;

arguments_opt
  : arguments
      { yTRACE("arguments_opt -> arguments \n");
      	$$ = $1;
      }
  |
      { yTRACE("arguments_opt -> \n");
      	$$ = NULL;
      }
  ;

%%

/***********************************************************************
 * Extra C code.
 *
 * The given yyerror function should not be touched. You may add helper
 * functions as necessary in subsequent phases.
 ***********************************************************************/
void yyerror(char* s) {
  if(errorOccurred) {
    return;    /* Error has already been reported by scanner */
  } else {
    errorOccurred = 1;
  }

  fprintf(errorFile, "\nPARSER ERROR, LINE %d", yyline);
  
  if(strcmp(s, "parse error")) {
    if(strncmp(s, "parse error, ", 13)) {
      fprintf(errorFile, ": %s\n", s);
    } else {
      fprintf(errorFile, ": %s\n", s+13);
    }
  } else {
    fprintf(errorFile, ": Reading token %s\n", yytname[YYTRANSLATE(yychar)]);
  }
}

