%{
#include "ast.hpp"
#include <iostream>
#include <memory>
#include <vector>

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
    std::vector<Param>* params;
    std::vector<std::unique_ptr<ASTNode>>* args;
}

%token <num> NUM
%token <str> ID
%token FN RETURN ARROW COMMA PRINT IF ELSE WHILE
%token EQ_ASSIGN NEQ_ASSIGN LOW_EQ HIGH_EQ AND OR LOW HIGH ADD SUB MUL DIV MOD
%token ASSIGN SEMICOLON LPAREN RPAREN LBRACE RBRACE

%left OR
%left AND
%left EQ_ASSIGN NEQ_ASSIGN LOW HIGH LOW_EQ HIGH_EQ
%left ADD SUB
%left MUL DIV MOD
%right NOT

%type <node> statement expr compound_statement fn_decl return_stmt call_expr
%type <block> statements
%type <params> param_list params
%type <args> arg_list args

%%

program:
    statements { rootBlock = std::unique_ptr<BlockNode>($1); }
    ;

statements:
    statements statement { $1->add_statement(std::unique_ptr<ASTNode>($2)); $$ = $1; }
    | statement { $$ = new BlockNode(); $$->add_statement(std::unique_ptr<ASTNode>($1)); }
    ;

statement:
    ID ASSIGN expr SEMICOLON { $$ = new AssignmentNode($1, std::unique_ptr<ASTNode>($3)); free($1); }
    | PRINT expr SEMICOLON { $$ = new PrintNode(std::unique_ptr<ASTNode>($2)); }
    | expr SEMICOLON { $$ = $1; }
    | IF LPAREN expr RPAREN compound_statement { $$ = new IfStmtNode(std::unique_ptr<ASTNode>($3), std::unique_ptr<ASTNode>($5)); }
    | IF LPAREN expr RPAREN compound_statement ELSE compound_statement { $$ = new IfStmtNode(std::unique_ptr<ASTNode>($3), std::unique_ptr<ASTNode>($5), std::unique_ptr<ASTNode>($7)); }
    | WHILE LPAREN expr RPAREN compound_statement { $$ = new WhileStmtNode(std::unique_ptr<ASTNode>($3), std::unique_ptr<ASTNode>($5)); }
    | fn_decl { $$ = $1; }
    | return_stmt { $$ = $1; }
    ;

compound_statement:
    LBRACE statements RBRACE { $$ = $2; }
    ;

fn_decl:
    FN ID LPAREN params RPAREN compound_statement {
        // Исправлено: индекс $6 вместо $7
        $$ = new FnDeclNode($2, *$4, std::unique_ptr<BlockNode>(static_cast<BlockNode*>($6)));
        delete $4; free($2);
    }
    ;

params:
    { $$ = new std::vector<Param>(); }
    | param_list { $$ = $1; }
    ;

param_list:
    ID { $$ = new std::vector<Param>(); $$->push_back({$1}); free($1); }
    | param_list COMMA ID { $1->push_back({$3}); $$ = $1; free($3); }
    ;

return_stmt:
    RETURN expr SEMICOLON { $$ = new ReturnStmtNode(std::unique_ptr<ASTNode>($2)); }
    ;

args:
    { $$ = new std::vector<std::unique_ptr<ASTNode>>(); }
    | arg_list { $$ = $1; }
    ;

arg_list:
    expr {
        $$ = new std::vector<std::unique_ptr<ASTNode>>();
        $$->push_back(std::unique_ptr<ASTNode>($1));
    }
    | arg_list COMMA expr {
        $1->push_back(std::unique_ptr<ASTNode>($3));
        $$ = $1;
    }
    ;

expr:
    NUM                     { $$ = new NumberNode($1); }
    | ID                    { $$ = new VariableNode($1); free($1); }
    | expr ADD expr         { $$ = new BinaryOpNode(BinOp::ADD, std::unique_ptr<ASTNode>($1), std::unique_ptr<ASTNode>($3)); }
    | expr SUB expr         { $$ = new BinaryOpNode(BinOp::SUB, std::unique_ptr<ASTNode>($1), std::unique_ptr<ASTNode>($3)); }
    | expr MUL expr         { $$ = new BinaryOpNode(BinOp::MUL, std::unique_ptr<ASTNode>($1), std::unique_ptr<ASTNode>($3)); }
    | expr DIV expr         { $$ = new BinaryOpNode(BinOp::DIV, std::unique_ptr<ASTNode>($1), std::unique_ptr<ASTNode>($3)); }
    | expr MOD expr         { $$ = new BinaryOpNode(BinOp::MOD, std::unique_ptr<ASTNode>($1), std::unique_ptr<ASTNode>($3)); }
    | expr EQ_ASSIGN expr   { $$ = new BinaryOpNode(BinOp::EQ_ASSIGN, std::unique_ptr<ASTNode>($1), std::unique_ptr<ASTNode>($3)); }
    | expr NEQ_ASSIGN expr  { $$ = new BinaryOpNode(BinOp::NEQ_ASSIGN, std::unique_ptr<ASTNode>($1), std::unique_ptr<ASTNode>($3)); }
    | expr LOW expr         { $$ = new BinaryOpNode(BinOp::LOW, std::unique_ptr<ASTNode>($1), std::unique_ptr<ASTNode>($3)); }
    | expr HIGH expr        { $$ = new BinaryOpNode(BinOp::HIGH, std::unique_ptr<ASTNode>($1), std::unique_ptr<ASTNode>($3)); }
    | expr LOW_EQ expr      { $$ = new BinaryOpNode(BinOp::LOW_EQ, std::unique_ptr<ASTNode>($1), std::unique_ptr<ASTNode>($3)); }
    | expr HIGH_EQ expr     { $$ = new BinaryOpNode(BinOp::HIGH_EQ, std::unique_ptr<ASTNode>($1), std::unique_ptr<ASTNode>($3)); }
    | expr AND expr         { $$ = new BinaryOpNode(BinOp::AND, std::unique_ptr<ASTNode>($1), std::unique_ptr<ASTNode>($3)); }
    | expr OR expr          { $$ = new BinaryOpNode(BinOp::OR, std::unique_ptr<ASTNode>($1), std::unique_ptr<ASTNode>($3)); }
    | NOT expr              { $$ = new UnaryOpNode(UnaryOp::NOT, std::unique_ptr<ASTNode>($2)); } // Исправлено: std::unique_ptr
    | LPAREN expr RPAREN    { $$ = $2; }
    | ID LPAREN args RPAREN { $$ = new CallExprNode($1, std::move(*$3)); delete $3; free($1); }
    ;

%%

void yyerror(const char* s) {
    std::cerr << "Parse error at line " << yylineno << ": " << s << std::endl;
}
