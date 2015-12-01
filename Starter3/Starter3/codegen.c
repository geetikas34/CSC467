#include "codegen.h"
#include "parser.tab.h"

FILE* frag;
bool available_temp_regs[NUM_TEMP_REGS];
bool previously_declared_temp[NUM_TEMP_REGS];

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
			//printf("Getting var%d\n",i);
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
		//printf("Freeing var%d\n", i);
	}
}
void decl_genCode(node* ast){
	// CONST
	/*if(ast->declaration.expr->kind == CONSTRUCTOR_NODE)
		genCode(ast->declaration.expr);*/
	char* tempvar = (char *)malloc(sizeof(char) * 1000);

	if(ast->declaration.constant){
		//only static assignment allowed: to a var or a literal or constructor(?)
		//for all?
		fprintf(frag,"PARAM %s = ", ast->declaration.id);
		if(ast->declaration.expr->kind == VAR_NODE || ast->declaration.expr->kind == CONSTRUCTOR_NODE){ //constructor???
			genCode(ast->declaration.expr, tempvar);
			fprintf(frag,"%s;\n", tempvar);
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

		fprintf(frag, "TEMP %s;\n",ast->declaration.id );
		//fprintf(frag, "TEMP tempVar%d;\n", dest);
		if(ast->declaration.expr != NULL ){
			if(ast->declaration.expr->kind ==CONSTRUCTOR_NODE){
				fprintf(frag, "MOV %s, ", ast->declaration.id);
				genCode(ast->declaration.expr);
			} else if(ast->declaration.expr->kind ==FUNCTION_NODE){
				genCode(ast->declaration.expr);
			}else{
				int dest = get_tempVar();
				genCode(ast->declaration.expr, tempvar);
				fprintf(frag, "MOV tempVar%d, %s;\n", dest, tempvar);
				free(tempvar);
			}
		}
	}
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
bool start = false;
void genCode(node* ast, char* tempVar, char* cond){
    if(ast == NULL)
    	return;
    int dest_temp;
    char exp_right[1000];
    char exp_left[1000];
    switch(ast->kind){
	case SCOPE_NODE:
		//fprintf(frag, "#SCOPE\n");
		genCode(ast->scope.decls);
		genCode(ast->scope.stats, tempVar, cond);
		break;
	case DECLARATIONS_NODE:
		//fprintf(frag, "#DECLS\n");
		genCode(ast->declarations.decls);
		genCode(ast->declarations.decl);
		break;
	case DECLARATION_NODE:
		//fprintf(frag, "#DECL\n");
		decl_genCode(ast);
		break;
	case STATEMENTS_NODE:
		genCode(ast->statements.stats, tempVar, cond);
		genCode(ast->statements.stat, tempVar, cond);
		break;
	case ASSIGNMENT_NODE:
		//genCode_assignment(ast);
		//fprintf(frag,"# assignment\n");
		//not looking at if_else for now
		genCode(ast->assignment.var, exp_left);
		genCode(ast->assignment.expr, exp_right);
		if(tempVar!=NULL){
			//if cond<0 --> false! so do opposite
			fprintf(frag,"CMP %s, %s, %s, %s;\n",exp_left, cond, exp_left, exp_right);
		} else {//normal assignment
			fprintf(frag, "MOV %s, %s;\n", exp_left, exp_right);
		}
		free_temp(exp_right);
		break;
	case VAR_NODE:
//		fprintf(frag, "#VAR_NODE\n");
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
		fprintf(frag,"# unnary op\n");
		dest_temp = get_tempVar();
		if(!previously_declared_temp[dest_temp]){
			fprintf(frag,"TEMP tempVar%d;\n", dest_temp);
			previously_declared_temp[dest_temp] = true;
		}
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
		fprintf(frag,"# binary op\n");
		dest_temp = get_tempVar();
		genCode(ast->binary_expr.left, exp_left);
		genCode(ast->binary_expr.right, exp_right);
		if(!previously_declared_temp[dest_temp]){
			fprintf(frag,"TEMP tempVar%d;\n", dest_temp);
			previously_declared_temp[dest_temp] = true;
		}
		switch(ast->binary_expr.op){
		case OR:
			// bitwise op
			//dest = left + right
			fprintf(frag, "ADD tempVar%d, %s, %s;\n", dest_temp, exp_right, exp_left);
			// dest = (dest<0)?-1:1
			fprintf(frag, "CMP tempVar%d, tempVar%d, -1.0, 1.0;\n", dest_temp, dest_temp);
			break;
		case AND:
			//dest = left + right
			fprintf(frag, "ADD tempVar%d, %s, %s;\n", dest_temp, exp_right, exp_left);
			// desp = dest -1
			//fprintf(frag, "ADD tempVar%d, tempVar%d, -1.0;\n", dest_temp, dest_temp);
			// dest = dest<0?-1:1
			fprintf(frag, "CMP tempVar%d, tempVar%d, -1.0, 1.0;\n", dest_temp, dest_temp);
			break;
		case EQ:
			//if(left-right)==0, then dest = 1; else dest = -1
			fprintf(frag, "SUB tempVar%d, %s, %s;\n", dest_temp, exp_left, exp_right);
			// get absolute (left-right)
			fprintf(frag, "ABS tempVar%d, tempVar%d;\n", dest_temp, dest_temp);
			//get -(left-right)
			fprintf(frag, "SUB tempVar%d, 0.0, tempVar%d;\n", dest_temp, dest_temp);
			// if(-abs(left-right)<0 => not equal
			fprintf(frag, "CMP tempVar%d, tempVar%d, -1.0, 1.0;\n", dest_temp, dest_temp);
			break;
		case NEQ:
			//if(left-right)==0, then dest = 1; else dest = -1
			fprintf(frag, "SUB tempVar%d, %s, %s;\n", dest_temp, exp_left, exp_right);
			// get absolute (left-right)
			fprintf(frag, "ABS tempVar%d, tempVar%d;\n", dest_temp, dest_temp);
			//get -(left-right)
			fprintf(frag, "SUB tempVar%d, 0.0, tempVar%d;\n", dest_temp, dest_temp);
			// if(-abs(left-right)<0 => not equal
			fprintf(frag, "CMP tempVar%d, tempVar%d, 1.0, -1.0;\n", dest_temp, dest_temp);
			break;
		case '<':
			// if left-right <0 => left < right (dest = 1.0)
			// dest = left-right
			fprintf(frag, "SUB tempVar%d, %s, %s;\n", dest_temp, exp_left, exp_right);
			//if dest <0, dest = 1
			fprintf(frag, "CMP tempVar%d, tempVar%d, 1.0, -1.0;\n", dest_temp, dest_temp);
			break;
		case LEQ:
			// if left-right<=0 OR right-left >=0 => left<=right (dest = 1)
			// dest = right-left
			fprintf(frag, "SUB tempVar%d, %s, %s;\n", dest_temp, exp_right, exp_left);
			//if dest <0, dest = -1 else dest = 1
			fprintf(frag, "CMP tempVar%d, tempVar%d, -1.0, 1.0;\n", dest_temp, dest_temp);
			break;
		case '>':
			// if right-left <0 => right < left (dest = 1.0)
			// dest = right-left
			fprintf(frag, "SUB tempVar%d, %s, %s;\n", dest_temp, exp_right, exp_left);
			//if dest <0, dest = 1
			fprintf(frag, "CMP tempVar%d, tempVar%d, 1.0, -1.0;\n", dest_temp, dest_temp);
			break;
		case GEQ:
			// if left-right >=0 => left>=right (dest = 1)
			// dest = left-right
			fprintf(frag, "SUB tempVar%d, %s, %s;\n", dest_temp,exp_left, exp_right);
			//if dest <0, dest = -1 else dest = 1
			fprintf(frag, "CMP tempVar%d, tempVar%d, -1.0, 1.0;\n", dest_temp, dest_temp);
			break;
		case '+':
			fprintf(frag, "ADD tempVar%d, %s, %s;\n", dest_temp, exp_left, exp_right);
			break;
		case '-':
			fprintf(frag, "SUB tempVar%d, %s, %s;\n", dest_temp, exp_left, exp_right);
			break;
		case '*':
			fprintf(frag, "MUL tempVar%d, %s, %s;\n", dest_temp, exp_left, exp_right);
			break;
		case '/':
			fprintf(frag, "RCP tempVar%d, %s;\n", dest_temp, exp_right);
			fprintf(frag, "MUL tempVar%d, %s, %s;\n", dest_temp, exp_left, exp_right);
			break;
		case '^':
			fprintf(frag, "POW tempVar%d, %s, %s;\n", dest_temp, exp_left, exp_right);
			break;
		default: fprintf(frag, "ERROR: Invalid binary op\n");
		}
		free_temp(exp_left);
		free_temp(exp_right);
		sprintf(tempVar, "tempVar%d", dest_temp);
		break;
	case ARGUMENT_NODE:
	//	fprintf(frag,"# arg\n");
		genCode(ast->arguments.args);
		genCode(ast->arguments.exprs, exp_right);
		if(!start){
			fprintf(frag," , ");
		}
		fprintf(frag, "%s",exp_right);
		start = false;
		break;
	case CONSTRUCTOR_NODE:
		fprintf(frag, "{");
		start = true;
		genCode(ast->constructor.args);
		fprintf(frag," }");
		break;
	case FUNCTION_NODE:
		fprintf(frag,"# FUNCTION\n");
		dest_temp = get_tempVar();
		switch(ast->function.function_name){
			case DP3:
				fprintf(frag, "DP3 ");
				break;
			case LIT:
				fprintf(frag, "LIT ");
				break;
			case RSQ:
				fprintf(frag, "RSQ ");
				break;
		}
		fprintf(frag, "tempVar%d, ", dest_temp);
		start = true;
		genCode(ast->function.args);
		sprintf(tempVar, "tempVar%d", dest_temp);
		fprintf(frag, ";\n");
		break;
	case IF_STATEMENT_NODE:
		//right = true/false
		fprintf(frag, "# IF/ELSE\n");
		genCode(ast->if_else.exprs, exp_right);
		dest_temp = get_tempVar();
		if(!previously_declared_temp[dest_temp]){
			fprintf(frag,"TEMP tempVar%d;\n", dest_temp);
			previously_declared_temp[dest_temp] = true;
		}
		fprintf(frag, "MOV tempVar%d, %s;\n", dest_temp, exp_right);
		sprintf(exp_right, "tempVar%d", dest_temp);
		//exp_right is if_cond. if true, exec if stmts

		if(ast->if_else.else_stats != NULL){
			dest_temp = get_tempVar();
			if(!previously_declared_temp[dest_temp]){
				fprintf(frag,"TEMP tempVar%d;\n", dest_temp);
				previously_declared_temp[dest_temp] = true;
			}
			// dest_temp is !if_cond i.e if dest_temp = TRUE, exec else stmts
			fprintf(frag, "CMP tempVar%d, %s,1.0, -1.0;\n", dest_temp, exp_right);
		}
		//if stmts
		//fprintf(frag, "#if stmts\n");
		genCode(ast->if_else.stats, "IFELSE", exp_right);
		if(ast->if_else.else_stats != NULL){
			//else stmts (exp_left = else condition)
			sprintf(exp_left, "tempVar%d", dest_temp);
			//fprintf(frag, "#else stmts\n");
			genCode(ast->if_else.else_stats, "IFELSE", exp_left);
		}
		break;
	default: printf("genCode: Not implemented node!\n");
	}
	
}


void init_codegen(node* ast){
	frag = fopen("frag.txt", "w");
	//frag = stdout;
	int i;
	for(i=0;i<NUM_TEMP_REGS;i++){
		available_temp_regs[i] = true;
		previously_declared_temp[i] = false;
	}
	fprintf(frag, "!!ARBfp1.0\n");
	genCode(ast);
	fprintf(frag, "END\n");
}
