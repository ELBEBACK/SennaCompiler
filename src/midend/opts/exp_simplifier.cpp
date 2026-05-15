#include "exp_simplifier.hpp"
#include <unordered_map>

static ConstantInt* as_const(Value* v) {
    return dynamic_cast<ConstantInt*>(v);
}

static Value* try_simplify(Function& fn, Instruction* inst) {
    if (inst->operands.size() != 2) return nullptr;

    Value* lhs = inst->operands[0];
    Value* rhs = inst->operands[1];
    ConstantInt* cl = as_const(lhs);
    ConstantInt* cr = as_const(rhs);

    if (lhs == rhs) {
        switch (inst->opcode) {
            case Opcode::SUB:     return fn.get_const(0);
            case Opcode::CMP_EQ:  return fn.get_const(1);
            case Opcode::CMP_NEQ: return fn.get_const(0);
            case Opcode::CMP_LT:  return fn.get_const(0);
            case Opcode::CMP_GT:  return fn.get_const(0);
            case Opcode::CMP_LEQ: return fn.get_const(1);
            case Opcode::CMP_GEQ: return fn.get_const(1);
            default: break;
        }
    }

    if (cr) {
        switch (inst->opcode) {
            case Opcode::ADD: if (cr->val == 0) return lhs;             break;
            case Opcode::SUB: if (cr->val == 0) return lhs;             break;
            case Opcode::MUL:
                if (cr->val == 1) return lhs;
                if (cr->val == 0) return fn.get_const(0);
                break;
            case Opcode::DIV: if (cr->val == 1) return lhs;             break;
            case Opcode::MOD: if (cr->val == 1) return fn.get_const(0); break;
            default: break;
        }
    }

    if (cl) {
        switch (inst->opcode) {
            case Opcode::ADD: if (cl->val == 0) return rhs;             break;
            case Opcode::MUL:
                if (cl->val == 1) return rhs;
                if (cl->val == 0) return fn.get_const(0);
                break;
            default: break;
        }
    }

    return nullptr;
}

bool AlgebraicSimplify::run(Function& fn) {
    std::unordered_map<Value*, Value*> repls;

    for (auto& bb : fn.blocks)
        for (auto* inst : bb->insts)
            if (Value* r = try_simplify(fn, inst))
                repls[inst] = r;

    if (repls.empty()) return false;

    for (auto& bb : fn.blocks)
        for (auto* inst : bb->insts)
            for (auto*& op : inst->operands) {
                auto it = repls.find(op);
                if (it != repls.end())
                    op = it->second;
            }

    return true;
}