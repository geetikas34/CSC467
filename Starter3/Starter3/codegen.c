#include "codegen.h"

FILE* frag;
bool available_temp_regs[NUM_TEMP_REGS];

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
	fprintf(frag, "ERROR: Out of regs!\n");
	return -1;
}


void free_temp(char * exp_right){
	if(strstr(exp_right, "tempVar")){

		char* a = exp_right[7];
		int i = a - '0';
		available_temp_regs[i] = true;
	}
}
void decl_genCode(node* ast){
	// CONST
	/*if(ast->declaration.expr->kind == CONSTRUCTOR_NODE)
		genCode(ast->declaration.expr);*/
	char* tempvar = (char *)malloc(sizeof(char) * 1000);
	int dest = get_tempVar();
	if(ast->declaration.constant){
		//only static assignment allowed: to a var or a literal or constructor(?)
		//for all?
		fprintf(frag,"PARAM tempVar%d = ", dest);
		if(ast->declaration.expr->kind == VAR_NODE || ast->declaration.expr->kind == CONSTRUCTOR_NODE){ //constructor???
			genCode(ast->declaration.expr, tempvar);
			fprintf(frag,"%s ;\n", tempvar);
		} else {
			switch(ast->declaration.expr->kind){
				case FLOAT_NODE:
					fprintf(frag, "%f\n", ast->declaration.expr->float_literal.val);
					break;
				case BOOL_NODE:
					fprintf(frag, "%d\n", ast->declaration.expr->bool_literal.val); //<-correct??
					break;
				case INT_NODE:
					fprintf(frag, "%d\n", ast->declaration.expr->int_literal.val);
					break;
				default: printf("decl_genCode: Shouldn't come here!\n");
			}
		}
	}//NOT CONST
	else {

		fprintf(frag, "TEMP tempVar%d;\n", dest);
		if(ast->declaration.expr != NULL ){
			if(ast->declaration.expr->kind !=CONSTRUCTOR_NODE){
				genCode(ast->declaration.expr, tempvar);
				fprintf(frag, "MOV tempVar%d, %s;\n", dest, tempvar);
			} else {
				fprintf(frag, "MOV tempVar%d, ", dest, tempvar);
				genCode(ast->declaration.expr);
			}
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
/*
void genCode_assignment(node* ast){
	//char* right = (char *)malloc(sizeof(char) * 1000);
//	char* left = (char *)malloc(sizeof(char) * 1000);

	fprintf(frag,"# assignment\n");
	//not looking at if_else for now
	genCode(ast->assignment.var, right);
	genCode(ast->assignment.expr, left);
	fprintf(frag, "MOV %s, %s;\n", left, right);
	free_temp(right);
	//free(right);
	//free(left);
}*/

void genCode(node* ast, char* tempVar){
    if(ast == NULL)
    	return;
    int dest_temp;
    char exp_right[1000];
    char exp_left[1000];
    switch(ast->kind){
	case SCOPE_NODE:
		fprintf(frag, "#SCOPE\n");
		genCode(ast->scope.decls);
		genCode(ast->scope.stats);
		break;
	case DECLARATIONS_NODE:
		fprintf(frag, "#DECLS\n");
		genCode(ast->declarations.decls);
		genCode(ast->declarations.decl);
		break;
	case DECLARATION_NODE:
		fprintf(frag, "#DECL\n");
		decl_genCode(ast);
		break;
	case STATEMENTS_NODE:
		genCode(ast->statements.stat);
		genCode(ast->statements.stats);
		break;
	case ASSIGNMENT_NODE:
		//genCode_assignment(ast);
		fprintf(frag,"# assignment\n");
		//not looking at if_else for now
		genCode(ast->assignment.var, exp_left);
		genCode(ast->assignment.expr, exp_right);
		fprintf(frag, "MOV %s, %s;\n", exp_left, exp_right);
		free_temp(exp_right);
		break;
	case VAR_NODE:
		fprintf(frag, "#VAR_NODE\n");
		get_var(ast, tempVar);
		break;
	case BOOL_NODE:
		if(ast->bool_literal.val)
			sprintf(tempVar, "%f", 1.0);
		else
			sprintf(tempVar,"%f", -1.0);
		break;
	case INT_NODE:
		sprintf(tempVar, "%d", ast->int_literal.val);
		break;
	case FLOAT_NODE:
		sprintf(tempVar, "%f", ast->float_literal.val);
		break;
	case UNARY_EXPRESSION_NODE:
		fprintf(frag,"# unary\n");
		dest_temp = get_tempVar();
		fprintf(frag,"TEMP tempVar%d;\n", dest_temp);
		genCode(ast->unary_expr.right,exp_right);
		switch(ast->unary_expr.op){
			case '-':
				fprintf(frag, "SUB tempVar%d, 0.0, %s;\n", dest_temp, exp_right);
				break;
			case '!':
				fprintf(frag, "NOT tempVar%d, %s;\n", dest_temp, exp_right);
				break;
			default: fprintf(frag, "ERROR: Invalid unary op\n");
		}
		sprintf(tempVar, "tempVar%d", dest_temp);
		free_temp(exp_right);
		break;
	case BINARY_EXPRESSION_NODE:
		fprintf(frag,"# binary\n");
		break;
	case ARGUMENT_NODE:
	//	fprintf(frag,"# arg\n");
		genCode(ast->arguments.args, "end");
		genCode(ast->arguments.exprs, exp_right);
		fprintf(frag, "%s",exp_right);
		if(strcmp(tempVar, "start") != 0)
			fprintf(frag," , ");
		break;
	case CONSTRUCTOR_NODE:
		fprintf(frag, "{");
		genCode(ast->constructor.args, "start");
		fprintf(frag," };\n");
		break;
	case FUNCTION_NODE:
		fprintf(frag,"# FUNCTION\n");
		/*switch(ast->function.function_name){
			case 0;
			break;

		}*/
		break;
	default: printf("genCode: Not implemented node!\n");
	}
	
}


void init_codegen(node* ast){
	//frag = fopen("frag.txt", "w");
	frag = stdout;
	// init special regs
//	fprintf(frag, "PARAM true = 1.0;\n");
//	fprintf(frag, "PARAM false = -1.0;\n");
	int i;
	for(i=0;i<NUM_TEMP_REGS;i++){
		available_temp_regs[i] = true;
	}
	fprintf(frag, "!!ARBfp1.0\n");
	genCode(ast);
	fprintf(frag, "END\n");
}
