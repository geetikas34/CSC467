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

void decl_genCode(node* ast){
	// CONST

	char tempvar[200];
	if(ast->declaration.constant){
		//only static assignment allowed: to a var or a literal or constructor(?)
		//for all?
		fprintf(assemblyFile,"PARAM %s = ", ast->declaration.id);
		if(ast->declaration.expr->kind == VAR_NODE || ast->declaration.expr->kind == CONSTRUCTOR_NODE){ //constructor???
			genCode(ast->declaration.expr);
			fprintf(assemblyFile,"\n");
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
			//free temp buf?
		}
	}
}
char* get_var(node* ast){
	int i;
	// predefined var
	for(i = 0; i < 13; i++){
		if((strcmp(ast->variable.id, GLSLcustomVar[i]) == 0)){
			fprintf(assemblyFile, "%s", ARBcustomVar[i]);
		}
	}
	//normal var
	if (i == 13){
		fprintf(assemblyFile, "%s", ast->variable.id);
	}

	// array element
	switch(ast->variable.array_index){
	case 0:
		fprintf(assemblyFile, ".x");
		break;
	case 1:
		fprintf(assemblyFile, ".y");
		break;
	case 2:
		fprintf(assemblyFile, ".z");
		break;
	case 3:
		fprintf(assemblyFile, ".w");
		break;
	case -1:
		break;
	default: printf("get_var: Shouldn't come here!\n");
	}
}
void genCode(node* ast, char* tempVar){
    if(ast == NULL)
    	return;

	switch(ast->kind){
	case SCOPE_NODE:
		genCode(ast->scope.decls);
		genCode(ast->scope.stats);
		break;
	case DECLARATIONS_NODE:
		genCode(ast->declarations.decls);
		genCode(ast->declarations.decl);
		break;
	case DECLARATION_NODE:
		decl_genCode(ast);
		break;
	case VAR_NODE:
		return get_var(ast);
		break;
	default: printf("genCode: Shouldn't come here!\n");
	}
	
}


void init_codegen(node* ast){
	assemblyFile = fopen("frag.txt", "w");
	genCode(ast);
}
