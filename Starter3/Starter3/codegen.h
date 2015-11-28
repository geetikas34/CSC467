#ifndef CODEGEN_H_
#define CODEGEN_H_
#include "ast.h"
void genCode(node* ast, char* temp = NULL);
void init_codegen(node* ast);
#endif /* CODEGEN_H_ */
