#include "const_fold_prop.hpp"

#include <unordered_map>


static bool is_foldable(Opcode op) {
    switch (op) {
        case Opcode::ADD:
        case Opcode::SUB:
        case Opcode::MUL:
        case Opcode::DIV:
        case Opcode::MOD:
        case Opcode::AND:
        case Opcode::OR:
        case Opcode::NOT:
        case Opcode::CMP_EQ:
        case Opcode::CMP_NEQ:
        case Opcode::CMP_LT:
        case Opcode::CMP_GT:
        case Opcode::CMP_LEQ:
        case Opcode::CMP_GEQ:
            return true;
        default:
            return false;
    }
}

static int64_t eval_binary(Opcode op, int64_t a, int64_t b) {
    switch (op) {
        case Opcode::ADD:     return a + b;
        case Opcode::SUB:     return a - b;
        case Opcode::MUL:     return a * b;
        case Opcode::DIV:     return b != 0 ? a / b : 0;
        case Opcode::MOD:     return b != 0 ? a % b : 0;
        case Opcode::AND:     return (a != 0 && b != 0) ? 1 : 0;
        case Opcode::OR:      return (a != 0 || b != 0) ? 1 : 0;
        case Opcode::CMP_EQ:  return a == b ? 1 : 0;
        case Opcode::CMP_NEQ: return a != b ? 1 : 0;
        case Opcode::CMP_LT:  return a <  b ? 1 : 0;
        case Opcode::CMP_GT:  return a >  b ? 1 : 0;
        case Opcode::CMP_LEQ: return a <= b ? 1 : 0;
        case Opcode::CMP_GEQ: return a >= b ? 1 : 0;
        default:              return 0;
    }
}

static ConstantInt* as_const(Value* v) {
    return dynamic_cast<ConstantInt*>(v);
}


bool ConstFold::run(Function& fn) {
    bool ever_changed = false;

    bool changed;
    do {
        changed = false;

        
        std::unordered_map<Value*, ConstantInt*> folds;

        for (auto& bb : fn.blocks) {
            for (auto* inst : bb->insts) {

                if (inst->opcode == Opcode::PHI) {
                    if (inst->operands.empty()) continue;

                    ConstantInt* first = as_const(inst->operands[0]);
                    if (!first) continue;

                    bool all_same = true;
                    for (auto* op : inst->operands) {
                        ConstantInt* c = as_const(op);
                        if (!c || c->val != first->val) { all_same = false; break; }
                    }
                    if (all_same)
                        folds[inst] = first;

                    continue;
                }

                if (!is_foldable(inst->opcode)) continue;

                if (inst->opcode == Opcode::NOT) {
                    if (inst->operands.size() != 1) continue;
                    ConstantInt* a = as_const(inst->operands[0]);
                    if (!a) continue;
                    folds[inst] = fn.get_const(a->val == 0 ? 1 : 0);
                    continue;
                }

                if (inst->operands.size() != 2) continue;
                ConstantInt* a = as_const(inst->operands[0]);
                ConstantInt* b = as_const(inst->operands[1]);
                if (!a || !b) continue;

                if ((inst->opcode == Opcode::DIV || inst->opcode == Opcode::MOD)
                    && b->val == 0)
                    continue;

                int64_t result = eval_binary(inst->opcode, a->val, b->val);
                folds[inst] = fn.get_const(result);
            }
        }

        if (folds.empty()) break;

        
        for (auto& bb : fn.blocks) {
            for (auto* inst : bb->insts) {
                for (auto*& op : inst->operands) {
                    auto it = folds.find(op);
                    if (it != folds.end()) {
                        op = it->second;
                        changed = true;
                    }
                }
            }
        }

        ever_changed |= changed;

    } while (changed);

    return ever_changed;
}