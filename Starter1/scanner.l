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
#include "math.h"

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
float_r     (([\+\-]?(([1-9][0-9]*)|0))\.[0-9]*)|([\+\-]?(\.[0-9]+))
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
                      yERROR("Wrong format of identifier");
                  } else if(num < -2097151 || num > 2097151) {
                      yERROR("Integer out of range");
                  } else {
                      yylval.num_int = num;
                      yTRACE(INT);
                      return INT;
                  }
               }

{float_r}      {
                  float f = atof(yytext);
                  if(f == HUGE_VAL) {
                     yERROR("Float out of range");
                  } else {
                     yTRACE(FLOAT);
                     return FLOAT;
                  }
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
                  yylval.num_int = *(yytext+4)-'0';
                  yTRACE(BVEC_TYPE);
                  return BVEC_TYPE;
               }

"ivec"[234]    {
                  yylval.num_int = *(yytext+4)-'0';
                  yTRACE(IVEC_TYPE);
                  return IVEC_TYPE;
               }

"vec"[234]     {
                  yylval.num_int = *(yytext+3)-'0';         
                  yTRACE(VEC_TYPE);
                  return VEC_TYPE;
               }

"const"        {
                  yTRACE(CONST);
                  return CONST;
               }

,              {
                  yTRACE(COMMA);
                  return COMMA;
               }

;              {
                  yTRACE(SEMICOLON);
                  return SEMICOLON;
               }

\{             {
                  yTRACE(OPEN_BRACE);
                  return OPEN_BRACE;
               }

\}             {
                  yTRACE(CLOSE_BRACE);
                  return CLOSE_BRACE;    
               }

\(             {
                  yTRACE(OPEN_PARENTHESES);                          
                  return OPEN_PARENTHESES;
               }

\)             {
                  yTRACE(CLOSE_PARENTHESES);
                  return CLOSE_PARENTHESES;
               }

\[             {
                  yTRACE(OPEN_SQUARE_BRACKET);
                  return OPEN_SQUARE_BRACKET;
               }

\]             {
                  yTRACE(CLOSE_SQUARE_BRACKET);
                  return CLOSE_SQUARE_BRACKET;
               }  

{operator}    {
                 yylval.str = strndup(yytext, yyleng);
                 yTRACE(OPERATOR);
                 return OPERATOR;
              }

"rsq"         {
                  yTRACE(RSQ);
                  return RSQ;
              }

"lit"         {
                  yTRACE(LIT);
                  return LIT;           
              }

"dpr"         {
                  yTRACE(DPR);
                  return DPR;       
              }

"if"          {
                 yTRACE(IF);
                 return IF;
              }
	          
"else"        {
                 yTRACE(ELSE);
                 return ELSE;
	      }

"while"       {
	         yTRACE(WHILE);
                 return WHILE;
              }

{identifier}  {
                 if (yyleng > 32) {
                   yERROR("Maximum identifier length exceeded");
                 }
                 else {
                   yylval.str = strndup(yytext, yyleng);
                   yTRACE(IDENTIFIER);
                   return IDENTIFIER;
                 }
              }

{WS}          {
              }

.             {
                 yERROR("Unknown token");
              }

%%
