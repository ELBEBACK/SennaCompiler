#include "ir_verify.hpp"
#include <algorithm>

bool IRValidator::validate(const Module& mod) {
    errors_.clear();
    for (const auto& fn : mod.functions) {
        verify_function(*fn);
    }
    return errors_.empty();
}

void IRValidator::verify_function(const Function& fn) {
    if (fn.blocks.empty()) {
        report("Function does not contain basic blocks", fn);
        return;
    }
    if (!fn.entry) {
        report("Function has no entry point", fn);
    }

    for (const auto& bb : fn.blocks) {
        verify_block(fn, *bb);
    }
}

void IRValidator::verify_block(const Function& fn, const BasicBlock& bb) {
    if (bb.insts.empty()) {
        report("Basic block '" + bb.label + "' is empty", fn, bb.label);
        return;
    }

    for (size_t i = 0; i < bb.insts.size(); ++i) {
        const auto* inst = bb.insts[i];
        bool is_term = inst->opcode == Opcode::JMP ||
                       inst->opcode == Opcode::BR ||
                       inst->opcode == Opcode::RET;

        if (is_term && i != bb.insts.size() - 1) {
            report("Terminator found in the middle of block", fn, bb.label);
        }
        if (i == bb.insts.size() - 1 && !is_term) {
            report("Block is not terminated", fn, bb.label);
        }

        verify_instruction(fn, bb, *inst);
    }

    for (auto* succ : bb.succs) {
        auto it = std::find(succ->preds.begin(), succ->preds.end(), &bb);
        if (it == succ->preds.end()) {
            report("CFG consistency broken: " + bb.label + " -> " + succ->label, fn, bb.label);
        }
    }
}

void IRValidator::verify_instruction(const Function& fn, const BasicBlock& bb, const Instruction& inst) {
    check_type_invariants(fn, bb, inst);
    check_use_def(fn, inst);
}

void IRValidator::check_type_invariants(const Function& fn, const BasicBlock& bb, const Instruction& inst) {
    auto op_count = inst.operands.size();

    switch (inst.opcode) {
        case Opcode::ALLOCA:
            if (op_count != 0) report("ALLOCA must not have operands", fn, bb.label);
            break;

        case Opcode::LOAD:
            if (op_count != 1) {
                report("LOAD requires 1 operand (address)", fn, bb.label);
            } else {
                auto* src = dynamic_cast<Instruction*>(inst.operands[0]);
                if (!src || src->opcode != Opcode::ALLOCA) {
                    report("LOAD operand must be an ALLOCA", fn, bb.label);
                }
            }
            break;

        case Opcode::STORE:
            if (op_count != 2) {
                report("STORE requires 2 operands (value and address)", fn, bb.label);
            } else {
                auto* dest = dynamic_cast<Instruction*>(inst.operands[1]);
                if (!dest || dest->opcode != Opcode::ALLOCA) {
                    report("STORE destination must be an ALLOCA", fn, bb.label);
                }
            }
            break;

        case Opcode::ADD: case Opcode::SUB: case Opcode::MUL:
        case Opcode::DIV: case Opcode::MOD: case Opcode::AND:
        case Opcode::OR:  case Opcode::CMP_EQ: case Opcode::CMP_NEQ:
        case Opcode::CMP_LT: case Opcode::CMP_GT: case Opcode::CMP_LEQ:
        case Opcode::CMP_GEQ:
            if (op_count != 2) report("Binary op requires 2 operands", fn, bb.label);
            break;

        case Opcode::BR:
            if (op_count != 1) report("BR requires condition operand", fn, bb.label);
            if (!inst.bb1 || !inst.bb2) report("BR requires two target blocks", fn, bb.label);
            break;

        case Opcode::JMP:
            if (!inst.bb1) report("JMP requires a target block", fn, bb.label);
            break;

        default: break;
    }
}

void IRValidator::check_use_def(const Function& fn, const Instruction& inst) {
    for (const auto* operand : inst.operands) {
        if (!operand) {
            report("Instruction has null operand", fn);
            continue;
        }

        bool found = false;
        for (const auto* p : fn.params) if (p == operand) found = true;

        if (!found) {
            for (const auto& v : fn.values) {
                if (v.get() == operand) {
                    found = true;
                    break;
                }
            }
        }

        if (!found) report("Operand value not defined in function scope", fn);
    }
}

void IRValidator::report(const std::string& msg, const Function& fn, const std::string& bb_lbl) {
    errors_.push_back({msg, fn.name, bb_lbl});
}
