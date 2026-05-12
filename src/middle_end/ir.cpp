#include "ir.hpp"

static bool has_result(Opcode op) {
    switch (op) {
        case Opcode::STORE:
        case Opcode::PRINT:
        case Opcode::JMP:
        case Opcode::BR:
        case Opcode::RET:
            return false;
        default:
            return true;
    }
}

std::string Function::fresh_temp() {
    return "%t" + std::to_string(temp_cnt_++);
}

BasicBlock* Function::new_block(const std::string& lbl) {
    auto bb   = std::make_unique<BasicBlock>();
    bb->id    = bb_cnt_++;
    bb->label = lbl.empty() ? "bb" + std::to_string(bb->id) : lbl;
    BasicBlock* p = bb.get();
    blocks.push_back(std::move(bb));
    return p;
}

ConstantInt* Function::get_const(int64_t v) {
    auto c = std::make_unique<ConstantInt>(v);
    ConstantInt* p = c.get();
    values.push_back(std::move(c));
    return p;
}

IrParam* Function::add_param(const std::string& n) {
    auto pv = std::make_unique<IrParam>(n);
    IrParam* p = pv.get();
    values.push_back(std::move(pv));
    params.push_back(p);
    return p;
}

Instruction* Function::emit_named(BasicBlock* bb, std::string nm, Opcode op, std::vector<Value*> ops) {
    auto i = std::make_unique<Instruction>(std::move(nm), op, std::move(ops));
    Instruction* p = i.get();
    values.push_back(std::move(i));
    bb->insts.push_back(p);
    return p;
}

Instruction* Function::emit(BasicBlock* bb, Opcode op, std::vector<Value*> ops) {
    std::string nm = has_result(op) ? fresh_temp() : "";
    return emit_named(bb, std::move(nm), op, std::move(ops));
}