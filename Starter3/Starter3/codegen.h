#ifndef CODEGEN_H_
#define CODEGEN_H_
#include "ast.h"
#define NUM_TEMP_REGS 16
bool available_temp_regs[NUM_TEMP_REGS] = {true};
void genCode(node* ast, char* temp = NULL);
void init_codegen(node* ast);
#endif /* CODEGEN_H_ */
