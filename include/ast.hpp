#pragma once
#include <memory>
#include <string>
#include <vector>

#include "op_types.hpp"

class NumberNode;
class VariableNode;
class BinaryOpNode;
class AssignmentNode;
class PrintNode;
class BlockNode;
class IfStmtNode;
class WhileStmtNode;
class UnaryOpNode;
class FnDeclNode;
class CallExprNode;
class ReturnStmtNode;

class IVisitor {
public:
    virtual ~IVisitor() = default;
    virtual void visit(NumberNode& node) = 0;
    virtual void visit(VariableNode& node) = 0;
    virtual void visit(BinaryOpNode& node) = 0;
    virtual void visit(AssignmentNode& node) = 0;
    virtual void visit(PrintNode& node) = 0;
    virtual void visit(BlockNode& node) = 0;
    virtual void visit(IfStmtNode& node) = 0;
    virtual void visit(WhileStmtNode& node) = 0;
    virtual void visit(UnaryOpNode& node) = 0;
    virtual void visit(FnDeclNode& node) = 0;
    virtual void visit(CallExprNode& node) = 0;
    virtual void visit(ReturnStmtNode& node) = 0;
};

class ASTNode {
public:
    virtual ~ASTNode() = default;
    virtual void accept(IVisitor& visitor) = 0;
};

class NumberNode : public ASTNode {
public:
    long long value;
    explicit NumberNode(long long val) : value(val) {}
    void accept(IVisitor& visitor) override { visitor.visit(*this); }
};

class VariableNode : public ASTNode {
public:
    std::string name;
    explicit VariableNode(const std::string& n) : name(n) {}
    void accept(IVisitor& visitor) override { visitor.visit(*this); }
};

class BinaryOpNode : public ASTNode {
public:
    BinOp op;
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;

    BinaryOpNode(BinOp op, std::unique_ptr<ASTNode> l, std::unique_ptr<ASTNode> r)
        : op(op), left(std::move(l)), right(std::move(r)) {}

    void accept(IVisitor& visitor) override { visitor.visit(*this); }
};

class AssignmentNode : public ASTNode {
public:
    std::string var_name;
    std::unique_ptr<ASTNode> expr;

    AssignmentNode(const std::string& name, std::unique_ptr<ASTNode> e)
        : var_name(name), expr(std::move(e)) {}

    void accept(IVisitor& visitor) override { visitor.visit(*this); }
};

class PrintNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> expr;

    explicit PrintNode(std::unique_ptr<ASTNode> e) : expr(std::move(e)) {}
    void accept(IVisitor& visitor) override { visitor.visit(*this); }
};

class BlockNode : public ASTNode {
public:
    std::vector<std::unique_ptr<ASTNode>> statements;

    void add_statement(std::unique_ptr<ASTNode> stmt) {
        statements.push_back(std::move(stmt));
    }
    void accept(IVisitor& visitor) override { visitor.visit(*this); }
};

class IfStmtNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> true_block;
    std::unique_ptr<ASTNode> false_block;

    IfStmtNode(std::unique_ptr<ASTNode> cond, std::unique_ptr<ASTNode> true_block, std::unique_ptr<ASTNode> false_block = nullptr)
        : condition(std::move(cond)), true_block(std::move(true_block)), false_block(std::move(false_block)) {}

    void accept(IVisitor& visitor) override { visitor.visit(*this); }
};

class WhileStmtNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> body;

    WhileStmtNode(std::unique_ptr<ASTNode> cond, std::unique_ptr<ASTNode> body)
        : condition(std::move(cond)), body(std::move(body)) {}

    void accept(IVisitor& visitor) override { visitor.visit(*this); }
};

class UnaryOpNode : public ASTNode {
public:
    UnaryOp op;
    std::unique_ptr<ASTNode> operand;

    UnaryOpNode(UnaryOp op, std::unique_ptr<ASTNode> operand)
        : op(op), operand(std::move(operand)) {}

    void accept(IVisitor& visitor) override { visitor.visit(*this); }
};

struct Param {
    std::string name;
};

class FnDeclNode : public ASTNode {
public:
    std::string name;
    std::vector<Param> params;
    std::unique_ptr<BlockNode> body;

    FnDeclNode(std::string name, std::vector<Param> params, std::unique_ptr<BlockNode> body)
        : name(std::move(name)), params(std::move(params)), body(std::move(body)) {}

    void accept(IVisitor& visitor) override { visitor.visit(*this); }
};

class CallExprNode : public ASTNode {
public:
    std::string function_name;
    std::vector<std::unique_ptr<ASTNode>> arguments;

    CallExprNode(std::string name, std::vector<std::unique_ptr<ASTNode>> args)
        : function_name(std::move(name)), arguments(std::move(args)) {}

    void accept(IVisitor& visitor) override { visitor.visit(*this); }
};

class ReturnStmtNode : public ASTNode {
public:
    std::unique_ptr<ASTNode> expr;
    explicit ReturnStmtNode(std::unique_ptr<ASTNode> e) : expr(std::move(e)) {}
    void accept(IVisitor& visitor) override { visitor.visit(*this); }
};
