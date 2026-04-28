#pragma once
#include "ast.hpp"
#include <ostream>
#include <iostream>
#include <string>

class GraphDump : public IVisitor {
    int node_cnt = 0;
    int cur_node_id = 0;
    std::ostream& os;
public:
    explicit GraphDump(std::ostream& os) : os(os) {}

    void header_write() const;
    void visit(NumberNode& node) override;
    void visit(VariableNode& node) override;
    void visit(BinaryOpNode& node) override;
    void visit(AssignmentNode& node) override;
    void visit(PrintNode& node) override;
    void visit(BlockNode& node) override;
    void visit(IfStmtNode& node) override;
    void visit(WhileStmtNode& node) override;
    void visit(FnDeclNode& node) override;
    void visit(UnaryOpNode& node) override;
    void visit(CallExprNode& node) override;
    void visit(ReturnStmtNode& node) override;
    void footer_write() const;
private:
    int  next_id() {return ++node_cnt;}
    void edge_write(int from, int to) const;
    std::string get_op(const BinOp& op) const;
};
