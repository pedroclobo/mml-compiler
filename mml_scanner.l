%option c++ prefix="mml_scanner_" outfile="mml_scanner.cpp"
%option stack noyywrap yylineno 8bit debug
%{ 
// make relevant includes before including the parser's tab file
#include <string>
#include <cdk/ast/sequence_node.h>
#include <cdk/ast/expression_node.h>
#include <cdk/ast/lvalue_node.h>
#include "mml_parser.tab.h"

// don't change this
#define yyerror LexerError
%}
%x X_STRING X_COMMENT X_ESCAPE

DIGIT                  [0-9]
INT                    0|[1-9]{DIGIT}*
DOUBLE                 {DIGIT}+"."{DIGIT}*|({DIGIT}*"."{DIGIT}+|{DIGIT}+{EXPONENT}){EXPONENT}?
EXPONENT               [eE][-+]?{DIGIT}+

%%
  yydebug=1; set_debug(1);

"//".*$                  ; /* ignore comments */

"/*"                     yy_push_state(X_COMMENT);
<X_COMMENT>"/*"          yy_push_state(X_COMMENT);
<X_COMMENT>"*/"          yy_pop_state();
<X_COMMENT>.|\n          ;

">="                     return tGE;
"<="                     return tLE;
"=="                     return tEQ;
"!="                     return tNE;
"&&"                     return tAND;
"||"                     return tOR;

"int"                    return tTYPE_INT;
"double"                 return tTYPE_DOUBLE;
"string"                 return tTYPE_STRING;
"void"                   return tTYPE_VOID;
"auto"                   return tTYPE_AUTO;

"public"                 return tPUBLIC;
"forward"                return tFORWARD;
"foreign"                return tFOREIGN;

"if"                     return tIF;
"elif"                   return tELIF;
"else"                   return tELSE;
"while"                  return tWHILE;
"stop"                   return tSTOP;
"next"                   return tNEXT;
"return"                 return tRETURN;

"!"                      return tPRINT;
"!!"                     return tPRINTLN;
"input"                  return tREAD;
"null"                   return tNULL;
"sizeof"                 return tSIZEOF;

"begin"                  return tBEGIN;
"end"                    return tEND;

"->"                     return tARROW;

[A-Za-z][A-Za-z0-9_]*    yylval.s = new std::string(yytext); return tIDENTIFIER;

\"                       yy_push_state(X_STRING); yylval.s = new std::string("");
<X_STRING>\"             yy_pop_state(); return tSTRING;
<X_STRING>\\             yy_push_state(X_ESCAPE);
<X_STRING>.              *yylval.s += *yytext;

<X_ESCAPE>\"             *yylval.s += '"'; yy_pop_state();
<X_ESCAPE>\\             *yylval.s += '\\'; yy_pop_state();
<X_ESCAPE>t              *yylval.s += '\t'; yy_pop_state();
<X_ESCAPE>n              *yylval.s += '\n'; yy_pop_state();
<X_ESCAPE>r              *yylval.s += '\r'; yy_pop_state();
<X_ESCAPE>[0-7]{1,3}     *yylval.s += (char)strtoul(yytext, NULL, 8); yy_pop_state();

0[0-7]+                  yylval.i = strtoul(yytext, NULL, 8); return tINTEGER;

{INT}                    yylval.i = strtol(yytext, nullptr, 10); return tINTEGER;
{DOUBLE}                 yylval.d = strtod(yytext, nullptr); return tDOUBLE;

[-()<>=+*/%;{}.,@~[\]?]  return *yytext;

[ \t\n\r]+               ; /* ignore whitespace */

.                        yyerror("Unknown character");

%%
