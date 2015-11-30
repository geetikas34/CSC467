#include "codegen.h"

FILE* assemblyFile;

char* GLSLcustomVar[13] = {
	"result.color",
	"result.depth",
	"fragment.position",
	"fragment.texcoord",
	"fragment.color",
	"fragment.color.secondary",
	"fragment.fogcoord",
	"state.light[0].half",
	"state.lightmodel.ambient",
	"state.material.shininess",
	"program.env[1]",
	"program.env[2]",
	"program.env[3]"
};

char* ARBcustomVar[13] = {
	"gl_FragColor",
	"gl_FragDepth",
	"gl_FragCoord",
        "gl_TexCoord",
	"gl_Color",
	"gl_Secondary",
	"gl_FogFragCoord",
	"gl_Light_Half",
	"gl_Light_Ambient",
	"gl_Material_Shininess",
	"env1",
	"env2",
	"env3"
};

int get_tempVar(){
	int i = 0;
	for(i = 0;i < NUM_TEMP_REGS; i++){
		if(available_temp_regs[i]){
			available_temp_regs[i] = false;
			return i;
		}
	}
	fprintf(assemblyFile, "ERROR: Out of regs!\n");
	return -1;
}


void free_temp(char * exp_right){
	if(strstr(exp_right, "tempVar")){
		int  i = atoi(exp_right[7]);
		available_temp_regs[i] = true;
	}
}
void decl_genCode(node* ast){
	// CONST
	char* tempvar = (char *)malloc(sizeof(char) * 1000);
	if(ast->declaration.constant){
		//only static assignment allowed: to a var or a literal or constructor(?)
		//for all?
		fprintf(assemblyFile,"PARAM %s = ;", ast->declaration.id);
		if(ast->declaration.expr->kind == VAR_NODE || ast->declaration.expr->kind == CONSTRUCTOR_NODE){ //constructor???
			genCode(ast->declaration.expr, tempvar);
			fprintf(assemblyFile,"%s ;\n", tempvar);
		} else {
			switch(ast->declaration.expr->kind){
				case FLOAT_NODE:
					fprintf(assemblyFile, "%f\n", ast->declaration.expr->float_literal.val);
					break;
				case BOOL_NODE:
					fprintf(assemblyFile, "%d\n", ast->declaration.expr->bool_literal.val); //<-correct??
					break;
				case INT_NODE:
					fprintf(assemblyFile, "%d\n", ast->declaration.expr->int_literal.val);
					break;
				default: printf("decl_genCode: Shouldn't come here!\n");
			}
		}
	}//NOT CONST
	else {
		fprintf(assemblyFile, "TEMP %s;\n", ast->declaration.id);
		if(ast->declaration.expr != NULL){
			genCode(ast->declaration.expr, tempvar);
			fprintf(assemblyFile, "MOV %s, %s;\n", ast->declaration.id, tempvar);
		}
	}
	free(tempvar);
}
char* get_var(node* ast, char* buf){
	int i;
	// predefined var
	for(i = 0; i < 13; i++){
		if((strcmp(ast->variable.id, ARBcustomVar[i]) == 0)){
			strcpy(buf, GLSLcustomVar[i]);
			break;
		}
	}
	//normal var
	if (i == 13){
		strcpy(buf, ast->variable.id);
	}

	// array element
	switch(ast->variable.array_index){
	case 0:
		strcat(buf, ".x");
		break;
	case 1:
		strcat(buf, ".y");
		break;
	case 2:
		strcat(buf, ".z");
		break;
	case 3:
		strcat(buf, ".w");
		break;
	case -1:
		break;
	default: printf("get_var: Shouldn't come here!\n");
	}
	return buf;
}
void genCode(node* ast, char* tempVar){
    if(ast == NULL)
    	return;
    int dest_temp;
    char* exp_right  =NULL;
	switch(ast->kind){
	case SCOPE_NODE:
		fprintf(assemblyFile, "#SCOPE\n");
		genCode(ast->scope.decls);
		genCode(ast->scope.stats);
		break;
	case DECLARATIONS_NODE:
		fprintf(assemblyFile, "#DECLS\n");
		genCode(ast->declarations.decls);
		genCode(ast->declarations.decl);
		break;
	case DECLARATION_NODE:
		fprintf(assemblyFile, "#DECL\n");
		decl_genCode(ast);
		break;
	case VAR_NODE:
		fprintf(assemblyFile, "#VAR_NODE\n");
		get_var(ast, tempVar);
		break;
	case BOOL_NODE:
		if(ast->bool_literal.val)
			sprintf(tempVar, "1");
		else
			sprintf(tempVar, "0");
		break;
	case INT_NODE:
		sprintf(tempVar, "%d", ast->int_literal.val);
		break;
	case FLOAT_NODE:
		sprintf(tempVar, "%f", ast->float_literal.val);
		break;
	case UNARY_EXPRESSION_NODE:
		dest_temp = get_tempVar();
		fprintf(assemblyFile,"TEMP tempVar%d;\n", dest_temp);
		genCode(ast->unary_expr.right,exp_right);
		switch(ast->unary_expr.op){
			case '-':
				fprintf(assemblyFile, "SUB tempVar%d, 0.0, %s;\n", dest_temp, exp_right);
				break;
			case '!':
				fprintf(assemblyFile, "NOT tempVar%d, %s;\n", dest_temp, exp_right);
				break;
			default: fprintf(assemblyFile, "ERROR: Invalid unary op\n");
		}
		sprintf(tempVar, "tempVar%d", dest_temp);
		free_temp(exp_right);
		break;
	case BINARY_EXPRESSION_NODE:
		break;
	default: printf("genCode: Shouldn't come here!\n");
	}
	
}


void init_codegen(node* ast){
	//assemblyFile = fopen("frag.txt", "w");
	assemblyFile = stdout;
	// init special regs
	fprintf(assemblyFile, "PARAM zero = 0;\n");
	fprintf(assemblyFile, "PARAM true = 1;\n");
	fprintf(assemblyFile, "PARAM false = 0;\n");
	genCode(ast);
}
