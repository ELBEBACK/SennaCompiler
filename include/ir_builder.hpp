#pragma once
#include <stack>
#include <string>
#include <unordered_map>

#include "ast.hpp"
#include "ir.hpp"

class IRBuilder : public IVisitor {
public:
    Module build(BlockNode& root);

private:
    Module      module_;
    Function*   cur_fn_   = nullptr;
    BasicBlock* cur_bb_   = nullptr;
    Value*      last_val_ = nullptr;

    std::unordered_map<std::string, Instruction*> allocas_;

    struct LoopCtx { BasicBlock* brk; BasicBlock* cont; };
    std::stack<LoopCtx> loop_stk_;

    bool         terminated() const;
    void         set_block(BasicBlock* bb);
    Instruction* alloca_of(const std::string& var);
    Value*       load_var(const std::string& var);
    void         store_var(const std::string& var, Value* val);
    void         setup_allocas(ASTNode* body, const std::vector<IrParam*>& params);

    void visit(NumberNode&)         override;
    void visit(VariableNode&)       override;
    void visit(BinaryOpNode&)       override;
    void visit(AssignmentNode&)     override;
    void visit(CompoundAssignNode&) override;
    void visit(UnaryOpNode&)        override;
    void visit(CallExprNode&)       override;
    void visit(PrintNode&)          override;
    void visit(BlockNode&)          override;
    void visit(IfStmtNode&)         override;
    void visit(WhileStmtNode&)      override;
    void visit(ForNode&)            override;
    void visit(BreakNode&)          override;
    void visit(ContinueNode&)       override;
    void visit(ReturnStmtNode&)     override;
    void visit(FnDeclNode&)         override;
};