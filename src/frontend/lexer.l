%{
#include "ast.hpp"
#include "parser.hpp"
#include <cstdlib>
#include <cstring>

extern "C" int yylex();
%}

%option noyywrap
%option yylineno

%%

[ \t\r\n]+              { }

"print"                 { return TOKEN_PRINT; }

[0-9]+                  {
                            yylval.num = std::atoi(yytext);
                            return TOKEN_NUMBER;
                        }

[a-zA-Z_][a-zA-Z0-9_]*  {
                            yylval.str = strdup(yytext);
                            return TOKEN_ID;
                        }

"+"                     { return '+'; }
"-"                     { return '-'; }
"*"                     { return '*'; }
"/"                     { return '/'; }
"="                     { return '='; }
";"                     { return ';'; }
"("                     { return '('; }
")"                     { return ')'; }

.                       { }

%%
