%option noyywrap
%option nounput
%option noinput

%{
#include <cstdlib>
#include <cstring>
#include "sysy.tab.hpp"
using namespace std;
%}

WS      [ \t\n\r]*
ID      [a-zA-Z_][a-zA-Z0-9_]*

INT     0|[1-9][0-9]*
HEX     0[xX][0-9a-fA-F]+
OCT     0[0-7]+

FLOAT   ([0-9]+\.[0-9]*|[0-9]*\.[0-9]+)([eE][+-]?[0-9]+)?|[0-9]+[eE][+-]?[0-9]+

%%

{WS}        ;
"//".*      ;

"int"       { return INT; }
"float"     { return FLOAT_T; }
"return"    { return RETURN; }

{ID}        {
                yylval.str_val = strdup(yytext);
                return IDENT;
            }

{FLOAT}     {
                yylval.str_val = strdup(yytext);
                return FLOAT_CONST;
            }

{HEX}       { yylval.int_val = strtol(yytext, nullptr, 16); return INT_CONST; }
{OCT}       { yylval.int_val = strtol(yytext, nullptr, 8);  return INT_CONST; }
{INT}       { yylval.int_val = strtol(yytext, nullptr, 10); return INT_CONST; }

.           { return yytext[0]; }

%%