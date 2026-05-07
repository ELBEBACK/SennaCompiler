#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>

#include "ast.hpp"

struct SymbolInfo {
    bool is_function;
};

class SemanticAnalyzer : public IVisitor {
private:
    std::vector<std::unordered_map<std::string, SymbolInfo>> scopes;
    bool has_error = false;
    int loop_depth = 0;

    void enter_scope();
    void exit_scope();

    void define_symbol(const std::string& name, bool is_function = false);
    bool resolve_symbol(const std::string& name);

    void report_error(const std::string& message);

public:
    SemanticAnalyzer();

    bool has_errors() const { return has_error; }

    void visit(NumberNode& node) override;
    void visit(VariableNode& node) override;
    void visit(BinaryOpNode& node) override;
    void visit(AssignmentNode& node) override;
    void visit(PrintNode& node) override;
    void visit(BlockNode& node) override;
    void visit(IfStmtNode& node) override;
    void visit(WhileStmtNode& node) override;
    void visit(UnaryOpNode& node) override;
    void visit(FnDeclNode& node) override;
    void visit(CallExprNode& node) override;
    void visit(ReturnStmtNode& node) override;
    void visit(BreakNode& node) override;
    void visit(ContinueNode& node) override;
};
