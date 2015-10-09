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

WS          [ \t\n]+
integer_r   [\+\-]?(([1-9][0-9]*)|0)
float_r     ([\+\-]?(([1-9][0-9]*)|0))\.[0-9]*
identifier  [_a-zA-Z][_a-zA-Z0-9]*
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
                  printf("In token BOOL val = %d", yylval.num_int);
                  yTRACE(BOOL);
                  return BOOL;
               }

{integer_r}    {
                  int c = yyinput();
                  bool invalid = false;
                  while(c!=' ' && c!='\t' && c!='\n' && c!=';') {
                     if((c>='a' && c<='z') || (c>='A' && c<='Z')) {
                        invalid = true;
                        break;
                     }
                     c = yyinput();
                  }
                  int num = atoi(yytext);
                  if (invalid) {
                        yERROR("Invalid identifier");
                  } else if(num < -2097152 || num > 2097151) {
                      yERROR("Integer out of bounds");
                  } else {
                      yylval.num_int = num;
                      printf("In token INT val = %d", yylval.num_int);
                      yTRACE(INT);
                      return INT;
                  }
               }

{float_r}      {
                  yylval.num_float = atof(yytext);
                  printf("In token FLOAT val = %f", yylval.num_float);
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
                  yylval.num_int = *(yytext+4)-'0';
                  printf("In token BVEC_TYPE val = %d", yylval.num_int);
                  yTRACE(BVEC_TYPE);
                  return BVEC_TYPE;
               }

"ivec"[234]    {
                  yylval.num_int = *(yytext+4)-'0';
                  printf("In token IVEC_TYPE val = %d", yylval.num_int);
                  yTRACE(IVEC_TYPE);
                  return IVEC_TYPE;
               }

"vec"[234]     {
                  yylval.num_int = *(yytext+3)-'0';
                  printf("In token VEC_TYPE val = %d", yylval.num_int);                 
                  yTRACE(VEC_TYPE);
                  return VEC_TYPE;
               }

"const"        {
                  printf("In token CONST ");
                  yTRACE(CONST);
                  return CONST;
               }

,              {
                  printf("In token COMMA ");
                  yTRACE(COMMA);
                  return COMMA;
               }

;              {
                  printf("In token SEMICOLON ");
                  yTRACE(SEMICOLON);
                  return SEMICOLON;
               }

\{             {
                  printf("In token OPEN_BRACE ");
                  yTRACE(OPEN_BRACE);
                  return OPEN_BRACE;
               }

\}             {
                  printf("In token CLOSE_BRACE ");
                  yTRACE(CLOSE_BRACE);
                  return CLOSE_BRACE;    
               }

\(             {
                  printf("In token OPEN_PARENTHESES ");
                  yTRACE(OPEN_PARENTHESES);                          
                  return OPEN_PARENTHESES;
               }

\)             {
                  printf("In token PARENTHESES_CLOSE ");
                  yTRACE(CLOSE_PARENTHESES);
                  return CLOSE_PARENTHESES;
               }

\[             {
                  printf("In token OPEN_SQUARE ");
                  yTRACE(OPEN_SQUARE_BRACKET);
                  return OPEN_SQUARE_BRACKET;
               }

\]             {
                  printf("In token CLOSE_SQUARE ");
                  yTRACE(CLOSE_SQUARE_BRACKET);
                  return CLOSE_SQUARE_BRACKET;
               }  

{operator}    {
                 yylval.str = strndup(yytext, yyleng);
                 printf("In token OPERATOR ");
                 yTRACE(OPERATOR);
                 return OPERATOR;
              }

"rsq"         {
                  printf("In token rsq");
                  yTRACE(RSQ);
                  return RSQ;
              }

"lit"         {
                  printf("In token lit");
                  yTRACE(LIT);
                  return LIT;           
              }

"dpr"         {
                  printf("In token dpr");
                  yTRACE(DPR);
                  return DPR;       
              }

"if"          {
                 printf("In token IF");
                 yTRACE(IF);
                 return IF;
	      }
"else"        {
                 printf("In token ELSE ");
                 yTRACE(ELSE);
                 return ELSE;
	      }

"while"       {
                 printf("In token WHILE ");
	         yTRACE(WHILE);
                 return WHILE;
              }

{identifier}  {
                yylval.str = strndup(yytext, yyleng);
                printf("In token IDENTIFIER val = %s", yylval.str);
                yTRACE(IDENTIFIER);
                return IDENTIFIER;
              }

{WS}          {
              }

.             {
                 yERROR("Unknown token");
              }

%%
