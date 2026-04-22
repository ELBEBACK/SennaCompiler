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
    long long num;
    char* str;
    ASTNode* node;
    BlockNode* block;
}

%token <num> NUM
%token <str> ID
%token PRINT IF ELSE WHILE
%token EQ_ASSIGN NEQ_ASSIGN LOW_EQ HIGH_EQ AND OR LOW HIGH ADD SUB MUL DIV MOD
%token ASSIGN SEMICOLON LPAREN RPAREN LBRACE RBRACE

%left OR
%left AND
%left EQ_ASSIGN NEQ_ASSIGN LOW HIGH LOW_EQ HIGH_EQ
%left ADD SUB
%left MUL DIV MOD

%type <node> statement expr compound_statement
%type <block> statements

%%

program:
    statements {
        rootBlock = std::unique_ptr<BlockNode>($1);
    }
    ;

statements:
    statements statement {
        $1->add_statement(std::unique_ptr<ASTNode>($2));
        $$ = $1;
    }
    | statement {
        $$ = new BlockNode();
        $$->add_statement(std::unique_ptr<ASTNode>($1));
    }
    ;

statement:
    ID ASSIGN expr SEMICOLON {
        $$ = new AssignmentNode($1, std::unique_ptr<ASTNode>($3));
        free($1);
    }
    | PRINT expr SEMICOLON {
        $$ = new PrintNode(std::unique_ptr<ASTNode>($2));
    }
    | expr SEMICOLON {
        $$ = $1;
    }
    | IF LPAREN expr RPAREN compound_statement {
        $$ = new IfStmtNode(std::unique_ptr<ASTNode>($3), std::unique_ptr<ASTNode>($5));
    }
    | IF LPAREN expr RPAREN compound_statement ELSE compound_statement {
        $$ = new IfStmtNode(std::unique_ptr<ASTNode>($3), std::unique_ptr<ASTNode>($5), std::unique_ptr<ASTNode>($7));
    }
    | WHILE LPAREN expr RPAREN compound_statement {
        $$ = new WhileStmtNode(std::unique_ptr<ASTNode>($3), std::unique_ptr<ASTNode>($5));
    }
    ;

compound_statement:
    LBRACE statements RBRACE {
        $$ = $2;
    }
    ;

expr:
    NUM { $$ = new NumberNode($1); }
    | ID   { $$ = new VariableNode($1); free($1); }
    | expr ADD expr { $$ = new BinaryOpNode(BinOp::ADD, std::unique_ptr<ASTNode>($1), std::unique_ptr<ASTNode>($3)); }
    | expr SUB expr { $$ = new BinaryOpNode(BinOp::SUB, std::unique_ptr<ASTNode>($1), std::unique_ptr<ASTNode>($3)); }
    | expr MUL expr { $$ = new BinaryOpNode(BinOp::MUL, std::unique_ptr<ASTNode>($1), std::unique_ptr<ASTNode>($3)); }
    | expr DIV expr { $$ = new BinaryOpNode(BinOp::DIV, std::unique_ptr<ASTNode>($1), std::unique_ptr<ASTNode>($3)); }
    | expr MOD expr { $$ = new BinaryOpNode(BinOp::MOD, std::unique_ptr<ASTNode>($1), std::unique_ptr<ASTNode>($3)); }
    | expr EQ_ASSIGN expr  { $$ = new BinaryOpNode(BinOp::EQ_ASSIGN, std::unique_ptr<ASTNode>($1), std::unique_ptr<ASTNode>($3)); }
    | expr NEQ_ASSIGN expr  { $$ = new BinaryOpNode(BinOp::NEQ_ASSIGN, std::unique_ptr<ASTNode>($1), std::unique_ptr<ASTNode>($3)); }
    | expr LOW expr       { $$ = new BinaryOpNode(BinOp::LOW, std::unique_ptr<ASTNode>($1), std::unique_ptr<ASTNode>($3)); }
    | expr HIGH expr       { $$ = new BinaryOpNode(BinOp::HIGH, std::unique_ptr<ASTNode>($1), std::unique_ptr<ASTNode>($3)); }
    | expr LOW_EQ expr  { $$ = new BinaryOpNode(BinOp::LOW_EQ, std::unique_ptr<ASTNode>($1), std::unique_ptr<ASTNode>($3)); }
    | expr HIGH_EQ expr  { $$ = new BinaryOpNode(BinOp::HIGH_EQ, std::unique_ptr<ASTNode>($1), std::unique_ptr<ASTNode>($3)); }
    | expr AND expr { $$ = new BinaryOpNode(BinOp::AND, std::unique_ptr<ASTNode>($1), std::unique_ptr<ASTNode>($3)); }
    | expr OR expr  { $$ = new BinaryOpNode(BinOp::OR, std::unique_ptr<ASTNode>($1), std::unique_ptr<ASTNode>($3)); }
    | LPAREN expr RPAREN        { $$ = $2; }
    ;

%%

void yyerror(const char* s) {
    std::cerr << "Parse error at line " << yylineno << ": " << s << std::endl;
}
