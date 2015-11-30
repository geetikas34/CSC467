#ifndef CODEGEN_H_
#define CODEGEN_H_
#include "ast.h"
#define TRUE "1.0"
#define FALSE "-1.0"
#define NUM_TEMP_REGS 16
void genCode(node* ast, char* temp = NULL);
void init_codegen(node* ast);
void genCode_assignment(node* ast);
#endif /* CODEGEN_H_ */
