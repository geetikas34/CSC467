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


#define YY_USER_INIT { yyin = inputFile; }
#define	yyinput      input
#define yTRACE(x)    { if (traceScanner) fprintf(traceFile, "TOKEN %3d : %s\n", x, yytext); }
#define yERROR(x)    { fprintf(errorFile, "\nLEXICAL ERROR, LINE %d: %s\n", yyline, x); errorOccurred = TRUE; yyterminate(); }

int CheckInt(void);
int yyline = 1;

%}
%option noyywrap

WS          [ \t\n]
alphabet    [a-zA-z]
digit       [0-9]
integer     [\+\-]?(((1-9)digit*)|0)
float       (integer).(digit)*
identifier  (_|alphabet)(alphabet|digit|_)*
operation   (\+|\-|\*|\/|&&|\|\||\<|\>|\<\=|\>\=|\=|\=\=|\!|\!\=|\^)

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
                  yylval->num_int = (strcmp("false",yytext) == 0)?0:1;
                  yTRACE(BOOL);
                  return BOOL;
               }

integer        {
                  yylval->num_int = atoi(yytext);
                  yTRACE(INT);
                  return INT;
               }

float          {
                  yylval->num_float = atof(yytext);
                  yTRACE(FLOAT);
                  return FLOAT;
               }

"bool"         {
                  yTRACE(BOOL_TYPE);
                  return BOOL_TYPE;
               }

"int"          {
                  yTRACE(INT_TYPE);
                  return INT_TYPE;
               }

"float"        {
                  yTRACE(FLOAT_TYPE);
                  return FLOAT_TYPE;
               }

"bvec"[234]    {
                  yylval->num_int = *(yytext+4);
                  yTRACE(BVEC_TYPE);
                  return BVEC_TYPE;
               }

"ivec"[234]    {
                  yylval->num_int = *(yytext+4);
                  yTRACE(IVEC_TYPE);
                  return IVEC_TYPE;
               }

"vec"[234]     {
                  yylval->num_float = *(yytext+3);
                  yTRACE(VEC_TYPE);
                  return VEC_TYPE;
               }

"const"        {
                  yTRACE(CONST);
                  return CONST;
               }

,             {
                  yTRACE(COMMA);
                  return COMMA;
              }

;             {
                  yTRACE(SEMICOLON);
                  return SEMICOLON;
              }

[\{\}]        {
                  yylval->num_int = *yytext;
                  yTRACE(CURLY_BRACKETS);
                  return CURLY_BRACKETS;
              }

[\(\)]        {
                  yylval->num_int = *yytext;
                  yTRACE(ROUND_BRACKETS);
                  return ROUND_BRACKETS;
              }

[\[\]]        {
                  yylval->num_int = *yytext;
                  yTRACE(SQUARE_BRACKETS);
                  return SQUARE_BRACKETS;
              }

operator      {
                 yylval->str = strdup(yytext);
                 yTRACE(OPERATOR);
                 return OPERATOR;
              }

identifier    {
                yylval->str = strdup(yytext);
                yTRACE(IDENTIFIER);
                return IDENTIFIER;
              }

"rsq"|"lit"|"dp3" {
                    yylval->str = strdup(yytext);
                    yTRACE(FUNCTION);
                    return FUNCTION;
                  }

"if"|"else"   {
                 yylval->num_int = (strcmp(yytext,"if")==0)?0:1;
                 yTRACE(IFELSE);
                 return IFELSE;
	      }

"while"       {
	         yTRACE(WHILE);
                 return WHILE;
	      }


%%
