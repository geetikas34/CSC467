%{
/**********************************************************************
 *
 * **YOUR GROUP INFO SHOULD GO HERE**
 *
 *  Interface to the scanner module for CSC467 course project
 **********************************************************************/


#include "common.h"
#include "parser.tab.h"
#include "string.h"
#include "stdlib.h"


#define YY_USER_INIT { yyin = inputFile; }
#define	yyinput      input
#define yTRACE(x)    { if (traceScanner) fprintf(traceFile, "TOKEN %3d : %s\n", x, yytext); }
#define yERROR(x)    { fprintf(errorFile, "\nLEXICAL ERROR, LINE %d: %s\n", yyline, x); errorOccurred = TRUE; yyterminate(); }

int CheckInt(void);
int yyline = 1;

%}
%option noyywrap

WS          [ \t]
integer_r     [\+\-]?(([1-9][0-9]*)|0)
float_r       ([\+\-]?(([1-9][0-9]*)|0)).[0-9]*
identifier  (_|[a-zA-z])([a-zA-z]|[0-9]|_)*
operator    (\+|\-|\*|\/|&&|\|\||\<|\>|\<\=|\>\=|\=|\=\=|\!|\!\=|\^)

%%
"/*"        {
              int c1      = 0;
              int c2      = yyinput();
              int curline = yyline;
              for(;;) {
                if (c2 == EOF) {
                  fprintf(errorFile, "\nLEXICAL ERROR, LINE %d: Unmatched /*\n", curline);
                  errorOccurred = TRUE;
                  yyterminate();
                }
                if (c1 == '*' && c2 == '/')
                  break;
                c1 = c2;
                c2 = yyinput();
                if (c1 == '\n' && c2 != EOF)
                  yyline++;
              }
            }

"false"|"true" {
                  yylval.num_int = (strcmp("false",yytext) == 0)?0:1;
                  printf("In token BOOL ");
                  yTRACE(BOOL);
                  return BOOL;
               }

{integer_r}        {
                  printf("In token INT ");
                  yylval.num_int = atoi(yytext);
                  yTRACE(INT);
                  return INT;
               }

{float_r}          {
                  printf("In token FLOAT ");
                  yylval.num_float = atof(yytext);
                  yTRACE(FLOAT);
                  return FLOAT;
               }

"bool"         {
                  printf("In token BOOL_TYPE ");
                  yTRACE(BOOL_TYPE);
                  return BOOL_TYPE;
               }

"int"          {
                  printf("In token INT_TYPE ");
                  yTRACE(INT_TYPE);
                  return INT_TYPE;
               }

"float"        {
                  printf("In token FLOAT_TYPE ");
                  yTRACE(FLOAT_TYPE);
                  return FLOAT_TYPE;
               }

"bvec"[234]    {
                  printf("In token BVEC_TYPE ");
                  yylval.num_int = *(yytext+4);
                  yTRACE(BVEC_TYPE);
                  return BVEC_TYPE;
               }

"ivec"[234]    {
                  printf("In token IVEC_TYPE ");
                  yylval.num_int = *(yytext+4);
                  yTRACE(IVEC_TYPE);
                  return IVEC_TYPE;
               }

"vec"[234]     {
                  printf("In token VEC_TYPE ");
                  yylval.num_float = *(yytext+3);
                  yTRACE(VEC_TYPE);
                  return VEC_TYPE;
               }

"const"        {
                  printf("In token CONST ");
                  yTRACE(CONST);
                  return CONST;
               }

,             {
                  printf("In token COMMA ");
                  yTRACE(COMMA);
                  return COMMA;
              }

;             {
                  printf("In token SEMICOLON ");
                  yTRACE(SEMICOLON);
                  return SEMICOLON;
              }

[\{\}]        {
                  printf("In token CURLY_BRACKETS ");
                  yylval.num_int = *yytext;
                  yTRACE(CURLY_BRACKETS);
                  return CURLY_BRACKETS;
              }

[\(\)]        {
                  printf("In token ROUND_BRACKET ");
                  yylval.num_int = *yytext;
                  yTRACE(ROUND_BRACKETS);
                  return ROUND_BRACKETS;
              }

[\[\]]        {
                 printf("In token SQUARE_BRACKETS ");
                  yylval.num_int = *yytext;
                  yTRACE(SQUARE_BRACKETS);
                  return SQUARE_BRACKETS;
              }
			     
{operator}      {
                  printf("In token OPERATOR ");
                 strcpy(yylval.str, yytext);
                 yTRACE(OPERATOR);
                 return OPERATOR;
              }

{identifier}    {
                printf("In token IDENTIFIER ");
                yylval.str = strdup(yytext);
                yTRACE(IDENTIFIER);
                return IDENTIFIER;
              }

"rsq"|"lit"|"dp3" {
                  printf("In token FUNCTION ");
                    yylval.str = strdup(yytext);
                    yTRACE(FUNCTION);
                    return FUNCTION;
                  }

"if"|"else"   {
                  printf("In token IFELSE ");
                 yylval.num_int = (strcmp(yytext,"if")==0)?0:1;
                 yTRACE(IFELSE);
                 return IFELSE;
	      }

"while"       {
                  printf("In token WHILE ");
	         yTRACE(WHILE);
                 return WHILE;
	      }

%%
