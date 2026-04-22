%{
#include "ast.hpp"
#include <iostream>
#include <memory>

extern "C" int yylex();
extern int yylineno;
void yyerror(const char* s);

std::unique_ptr<BlockNode> rootBlock;
%}

%union {
    int num;
    char* str;
    ASTNode* node;
}

%token <num> TOKEN_NUMBER
%token <str> TOKEN_ID
%token TOKEN_PRINT

%type <node> statement expr

%left '+' '-'
%left '*' '/'

%%

program:
    statements { }
    ;

statements:
    statements statement { rootBlock->add_statement(std::unique_ptr<ASTNode>($2)); }
    | statement          { rootBlock->add_statement(std::unique_ptr<ASTNode>($1)); }
    ;

statement:
    TOKEN_ID '=' expr ';' {
        $$ = new AssignmentNode($1, std::unique_ptr<ASTNode>($3));
        free($1);
    }
    | TOKEN_PRINT expr ';' {
        $$ = new PrintNode(std::unique_ptr<ASTNode>($2));
    }
    | expr ';' {
        $$ = $1;
    }
    ;

expr:
    TOKEN_NUMBER {
        $$ = new NumberNode($1);
    }
    | TOKEN_ID {
        $$ = new VariableNode($1);
        free($1);
    }
    | expr '+' expr { $$ = new BinaryOpNode('+', std::unique_ptr<ASTNode>($1), std::unique_ptr<ASTNode>($3)); }
    | expr '-' expr { $$ = new BinaryOpNode('-', std::unique_ptr<ASTNode>($1), std::unique_ptr<ASTNode>($3)); }
    | expr '*' expr { $$ = new BinaryOpNode('*', std::unique_ptr<ASTNode>($1), std::unique_ptr<ASTNode>($3)); }
    | expr '/' expr { $$ = new BinaryOpNode('/', std::unique_ptr<ASTNode>($1), std::unique_ptr<ASTNode>($3)); }
    | '(' expr ')'  { $$ = $2; }
    ;

%%

void yyerror(const char* s) {
    std::cerr << "Parse error at line " << yylineno << ": " << s << std::endl;
}
