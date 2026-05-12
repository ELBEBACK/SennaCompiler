#include "ir_builder.hpp"
#include <unordered_set>

class VarCollector : public IVisitor {
public:
    std::unordered_set<std::string> vars;

    void visit(AssignmentNode& n)     override { vars.insert(n.var_name); n.expr->accept(*this); }
    void visit(CompoundAssignNode& n) override { vars.insert(n.var_name); n.expr->accept(*this); }
    void visit(UnaryOpNode& n)        override { n.operand->accept(*this); }
    void visit(FnDeclNode&)           override {}
    void visit(NumberNode&)           override {}
    void visit(VariableNode&)         override {}
    void visit(BreakNode&)            override {}
    void visit(ContinueNode&)         override {}
    void visit(BinaryOpNode& n)       override { n.left->accept(*this); n.right->accept(*this); }
    void visit(PrintNode& n)          override { n.expr->accept(*this); }
    void visit(ReturnStmtNode& n)     override { if (n.expr) n.expr->accept(*this); }
    void visit(CallExprNode& n)       override { for (auto& a : n.arguments) a->accept(*this); }

    void visit(BlockNode& n) override {
        for (auto& s : n.statements) s->accept(*this);
    }
    void visit(IfStmtNode& n) override {
        n.condition->accept(*this);
        n.true_block->accept(*this);
        if (n.false_block) n.false_block->accept(*this);
    }
    void visit(WhileStmtNode& n) override {
        n.condition->accept(*this);
        n.body->accept(*this);
    }
    void visit(ForNode& n) override {
        n.init->accept(*this); n.cond->accept(*this);
        n.step->accept(*this); n.body->accept(*this);
    }
};

static Opcode binop_opcode(BinOp op) {
    switch (op) {
        case BinOp::ADD:        return Opcode::ADD;
        case BinOp::SUB:        return Opcode::SUB;
        case BinOp::MUL:        return Opcode::MUL;
        case BinOp::DIV:        return Opcode::DIV;
        case BinOp::MOD:        return Opcode::MOD;
        case BinOp::AND:        return Opcode::AND;
        case BinOp::OR:         return Opcode::OR;
        case BinOp::EQ_ASSIGN:  return Opcode::CMP_EQ;
        case BinOp::NEQ_ASSIGN: return Opcode::CMP_NEQ;
        case BinOp::LOW:        return Opcode::CMP_LT;
        case BinOp::HIGH:       return Opcode::CMP_GT;
        case BinOp::LOW_EQ:     return Opcode::CMP_LEQ;
        case BinOp::HIGH_EQ:    return Opcode::CMP_GEQ;
        default:                return Opcode::ADD;
    }
}

static Opcode compound_arith(BinOp op) {
    switch (op) {
        case BinOp::ADD_ASSIGN: return Opcode::ADD;
        case BinOp::SUB_ASSIGN: return Opcode::SUB;
        case BinOp::MUL_ASSIGN: return Opcode::MUL;
        case BinOp::DIV_ASSIGN: return Opcode::DIV;
        case BinOp::MOD_ASSIGN: return Opcode::MOD;
        default:                return Opcode::ADD;
    }
}

bool IRBuilder::terminated() const { return cur_bb_->is_terminated(); }
void IRBuilder::set_block(BasicBlock* bb) { cur_bb_ = bb; }

Instruction* IRBuilder::alloca_of(const std::string& var) {
    auto it = allocas_.find(var);
    return it != allocas_.end() ? it->second : nullptr;
}

Value* IRBuilder::load_var(const std::string& var) {
    return cur_fn_->emit(cur_bb_, Opcode::LOAD, {alloca_of(var)});
}

void IRBuilder::store_var(const std::string& var, Value* val) {
    cur_fn_->emit(cur_bb_, Opcode::STORE, {val, alloca_of(var)});
}

void IRBuilder::setup_allocas(ASTNode* body, const std::vector<IrParam*>& params) {
    VarCollector vc;
    body->accept(vc);
    for (auto* p : params) vc.vars.insert(p->name);

    for (auto& var : vc.vars)
        allocas_[var] = cur_fn_->emit_named(cur_bb_, "%" + var, Opcode::ALLOCA, {});

    for (auto* p : params)
        store_var(p->name, p);
}

Module IRBuilder::build(BlockNode& root) {
    auto fn  = std::make_unique<Function>();
    fn->name = "main";
    cur_fn_  = fn.get();

    cur_fn_->entry = cur_fn_->new_block("entry");
    set_block(cur_fn_->entry);
    setup_allocas(&root, {});
    root.accept(*this);

    if (!terminated())
        cur_fn_->emit(cur_bb_, Opcode::RET, {});

    cur_fn_->prune_dead_blocks();
    module_.functions.push_back(std::move(fn));
    return std::move(module_);
}

void IRBuilder::visit(NumberNode& node) {
    last_val_ = cur_fn_->get_const(node.value);
}

void IRBuilder::visit(VariableNode& node) {
    last_val_ = load_var(node.name);
}

void IRBuilder::visit(BinaryOpNode& node) {
    node.left->accept(*this);  Value* lhs = last_val_;
    node.right->accept(*this); Value* rhs = last_val_;
    last_val_ = cur_fn_->emit(cur_bb_, binop_opcode(node.op), {lhs, rhs});
}

void IRBuilder::visit(AssignmentNode& node) {
    node.expr->accept(*this);
    store_var(node.var_name, last_val_);
    last_val_ = load_var(node.var_name);
}

void IRBuilder::visit(CompoundAssignNode& node) {
    Value* cur = load_var(node.var_name);
    node.expr->accept(*this);
    Value* res = cur_fn_->emit(cur_bb_, compound_arith(node.op), {cur, last_val_});
    store_var(node.var_name, res);
    last_val_ = res;
}

void IRBuilder::visit(UnaryOpNode& node) {
    node.operand->accept(*this);
    Value* src = last_val_;

    if (node.op == UnaryOp::INC || node.op == UnaryOp::DEC) {
        Opcode arith = node.op == UnaryOp::INC ? Opcode::ADD : Opcode::SUB;
        Value* res   = cur_fn_->emit(cur_bb_, arith, {src, cur_fn_->get_const(1)});
        auto*  slot  = static_cast<Instruction*>(src)->operands[0];
        cur_fn_->emit(cur_bb_, Opcode::STORE, {res, slot});
        last_val_ = res;
    } else {
        last_val_ = cur_fn_->emit(cur_bb_, Opcode::NOT, {src});
    }
}

void IRBuilder::visit(CallExprNode& node) {
    std::vector<Value*> args;
    for (auto& arg : node.arguments) {
        arg->accept(*this);
        args.push_back(last_val_);
    }
    auto* call    = cur_fn_->emit(cur_bb_, Opcode::CALL, std::move(args));
    call->fn_name = node.function_name;
    last_val_     = call;
}

void IRBuilder::visit(PrintNode& node) {
    node.expr->accept(*this);
    cur_fn_->emit(cur_bb_, Opcode::PRINT, {last_val_});
}

void IRBuilder::visit(BlockNode& node) {
    for (auto& stmt : node.statements) {
        if (terminated()) break;
        stmt->accept(*this);
    }
}

void IRBuilder::visit(ReturnStmtNode& node) {
    if (node.expr) {
        node.expr->accept(*this);
        cur_fn_->emit(cur_bb_, Opcode::RET, {last_val_});
    } else {
        cur_fn_->emit(cur_bb_, Opcode::RET, {});
    }
    set_block(cur_fn_->new_block("dead"));
}

void IRBuilder::visit(IfStmtNode& node) {
    node.condition->accept(*this);

    auto* true_bb  = cur_fn_->new_block("if.true");
    auto* false_bb = cur_fn_->new_block("if.false");
    auto* merge_bb = cur_fn_->new_block("if.merge");

    auto* br = cur_fn_->emit(cur_bb_, Opcode::BR, {last_val_});
    br->bb1  = true_bb;
    br->bb2  = false_bb;
    cur_bb_->add_edge(true_bb);
    cur_bb_->add_edge(false_bb);

    set_block(true_bb);
    node.true_block->accept(*this);
    if (!true_bb->is_terminated()) {
        cur_fn_->emit(true_bb, Opcode::JMP, {})->bb1 = merge_bb;
        true_bb->add_edge(merge_bb);
    }

    set_block(false_bb);
    if (node.false_block) node.false_block->accept(*this);
    if (!false_bb->is_terminated()) {
        cur_fn_->emit(false_bb, Opcode::JMP, {})->bb1 = merge_bb;
        false_bb->add_edge(merge_bb);
    }

    set_block(merge_bb);
}

void IRBuilder::visit(WhileStmtNode& node) {
    auto* header_bb = cur_fn_->new_block("while.header");
    auto* body_bb   = cur_fn_->new_block("while.body");
    auto* exit_bb   = cur_fn_->new_block("while.exit");

    cur_fn_->emit(cur_bb_, Opcode::JMP, {})->bb1 = header_bb;
    cur_bb_->add_edge(header_bb);

    set_block(header_bb);
    node.condition->accept(*this);
    auto* br = cur_fn_->emit(header_bb, Opcode::BR, {last_val_});
    br->bb1  = body_bb;
    br->bb2  = exit_bb;
    header_bb->add_edge(body_bb);
    header_bb->add_edge(exit_bb);

    loop_stk_.push({exit_bb, header_bb});
    set_block(body_bb);
    node.body->accept(*this);
    if (!terminated()) {
        cur_fn_->emit(cur_bb_, Opcode::JMP, {})->bb1 = header_bb;
        cur_bb_->add_edge(header_bb);
    }
    loop_stk_.pop();

    set_block(exit_bb);
}

void IRBuilder::visit(ForNode& node) {
    node.init->accept(*this);

    auto* header_bb = cur_fn_->new_block("for.header");
    auto* body_bb   = cur_fn_->new_block("for.body");
    auto* step_bb   = cur_fn_->new_block("for.step");
    auto* exit_bb   = cur_fn_->new_block("for.exit");

    cur_fn_->emit(cur_bb_, Opcode::JMP, {})->bb1 = header_bb;
    cur_bb_->add_edge(header_bb);

    set_block(header_bb);
    node.cond->accept(*this);
    auto* br = cur_fn_->emit(header_bb, Opcode::BR, {last_val_});
    br->bb1  = body_bb;
    br->bb2  = exit_bb;
    header_bb->add_edge(body_bb);
    header_bb->add_edge(exit_bb);

    loop_stk_.push({exit_bb, step_bb});
    set_block(body_bb);
    node.body->accept(*this);
    if (!terminated()) {
        cur_fn_->emit(cur_bb_, Opcode::JMP, {})->bb1 = step_bb;
        cur_bb_->add_edge(step_bb);
    }
    loop_stk_.pop();

    set_block(step_bb);
    node.step->accept(*this);
    if (!terminated()) {
        cur_fn_->emit(step_bb, Opcode::JMP, {})->bb1 = header_bb;
        step_bb->add_edge(header_bb);
    }

    set_block(exit_bb);
}

void IRBuilder::visit(BreakNode&) {
    cur_fn_->emit(cur_bb_, Opcode::JMP, {})->bb1 = loop_stk_.top().brk;
    cur_bb_->add_edge(loop_stk_.top().brk);
    set_block(cur_fn_->new_block("dead"));
}

void IRBuilder::visit(ContinueNode&) {
    cur_fn_->emit(cur_bb_, Opcode::JMP, {})->bb1 = loop_stk_.top().cont;
    cur_bb_->add_edge(loop_stk_.top().cont);
    set_block(cur_fn_->new_block("dead"));
}

void IRBuilder::visit(FnDeclNode& node) {
    Function*   s_fn     = cur_fn_;
    BasicBlock* s_bb     = cur_bb_;
    auto        s_alloca = std::move(allocas_);

    auto fn  = std::make_unique<Function>();
    fn->name = node.name;
    cur_fn_  = fn.get();
    allocas_.clear();

    cur_fn_->entry = cur_fn_->new_block("entry");
    set_block(cur_fn_->entry);

    for (auto& p : node.params) cur_fn_->add_param(p.name);
    setup_allocas(node.body.get(), cur_fn_->params);
    node.body->accept(*this);

    if (!terminated())
        cur_fn_->emit(cur_bb_, Opcode::RET, {});

    cur_fn_->prune_dead_blocks();
    module_.functions.push_back(std::move(fn));

    cur_fn_  = s_fn;
    cur_bb_  = s_bb;
    allocas_ = std::move(s_alloca);
}