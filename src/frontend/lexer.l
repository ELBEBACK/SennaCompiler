%{
#include "ast.hpp"
#include "parser.hpp"
#include <cstdlib>
#include <cstring>
#include <iostream>

extern "C" int yylex();
%}

%option noyywrap
%option yylineno

%x COMMENT

%%

[ \t\r\n]+              { }

"print"                 { return PRINT; }
"if"                    { return IF; }
"else"                  { return ELSE; }
"while"                 { return WHILE; }
"func"                  { return FN; }
"return"                { return RETURN; }

[0-9]+                  {
                            yylval.num = std::atoll(yytext);
                            return NUM;
                        }

[a-zA-Z_][a-zA-Z0-9_]*  {
                            yylval.str = strdup(yytext);
                            return ID;
                        }

"+="                    { return ADD_ASSIGN; }
"-="                    { return SUB_ASSIGN; }
"*="                    { return MUL_ASSIGN; }
"/="                    { return DIV_ASSIGN; }
"%="                    { return MOD_ASSIGN; }
"+"                     { return ADD; }
"-"                     { return SUB; }
"*"                     { return MUL; }
"/"                     { return DIV; }
"="                     { return ASSIGN; }
";"                     { return SEMICOLON; }
"("                     { return LPAREN; }
")"                     { return RPAREN; }
"&&"                    { return AND; }
"||"                    { return OR; }
"%"                     { return MOD; }
"!"                     { return NOT; }
"=="                    { return EQ_ASSIGN; }
"!="                    { return NEQ_ASSIGN; }
"<="                    { return LOW_EQ; }
">="                    { return HIGH_EQ; }
"<"                     { return LOW; }
">"                     { return HIGH; }
"{"                     { return LBRACE; }
"}"                     { return RBRACE; }
","                     { return COMMA; }
"//".*                  {}
"/*"                    {BEGIN(COMMENT);}
<COMMENT>"*/"           {BEGIN(INITIAL);}
<COMMENT>\n             {}
<COMMENT>.              {}

.                       { std::cerr << "Unknown character: "
                        << yytext << " at line " << yylineno << std::endl; }

%%
