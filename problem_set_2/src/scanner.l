%{
#include <vslc.h>
// The tokens defined in parser.y
#include "parser.h"

// parser.h contains some unused functions, ignore that
#pragma GCC diagnostic ignored "-Wunused-function"
%}
%option noyywrap
%option array
%option yylineno

WHITESPACE [\ \t\v\r\n]
COMMENT \/\/[^\n]+
QUOTED \"([^\"\n]|\\\")*\"


%%
{WHITESPACE}+           { /* Eliminate whitespace */ }
{COMMENT}               { /* Eliminate comments */ }
{QUOTED}                { return STRING; }

"if"                      { return IF; }
"else"                    { return ELSE; }
"while"                   { return WHILE; }
"return"                  { return RETURN; }
"func"                    { return FUNC; }
"print"                   { return PRINT; }
"break"                   { return BREAK; }
"then"                    { return THEN; }
"do"                      { return DO; }
"var"                     { return VAR; }
"begin"               { return OPENBLOCK; }
"end"              { return CLOSEBLOCK; }

[A-Za-z_][A-Za-z0-9_]* { return IDENTIFIER; }

[0-9]+                  { return NUMBER; }

.                       { return yytext[0]; }
%%
