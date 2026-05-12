#include "ir_print.hpp"

static std::string opcode_str(Opcode op) {
    switch (op) {
        case Opcode::ADD:     return "add";
        case Opcode::SUB:     return "sub";
        case Opcode::MUL:     return "mul";
        case Opcode::DIV:     return "div";
        case Opcode::MOD:     return "mod";
        case Opcode::AND:     return "and";
        case Opcode::OR:      return "or";
        case Opcode::NOT:     return "not";
        case Opcode::CMP_EQ:  return "cmp.eq";
        case Opcode::CMP_NEQ: return "cmp.neq";
        case Opcode::CMP_LT:  return "cmp.lt";
        case Opcode::CMP_GT:  return "cmp.gt";
        case Opcode::CMP_LEQ: return "cmp.leq";
        case Opcode::CMP_GEQ: return "cmp.geq";
        default:              return "?";
    }
}

static std::string val_str(const Value* v) {
    if (!v) return "<null>";
    if (auto* c = dynamic_cast<const ConstantInt*>(v))
        return std::to_string(c->val);
    return v->name;
}

void IRPrinter::print(const Module& mod) {
    for (auto& fn : mod.functions) {
        print_fn(*fn);
        os_ << "\n";
    }
}

void IRPrinter::print_fn(const Function& fn) {
    os_ << "func @" << fn.name << "(";
    for (size_t i = 0; i < fn.params.size(); ++i)
        os_ << fn.params[i]->name << (i + 1 < fn.params.size() ? ", " : "");
    os_ << "):\n";

    for (auto& bb : fn.blocks) print_bb(*bb);
}

void IRPrinter::print_bb(const BasicBlock& bb) {
    os_ << bb.label << ":";
    if (!bb.preds.empty()) {
        os_ << "                    ; preds:";
        for (auto* p : bb.preds) os_ << " " << p->label;
    }
    os_ << "\n";
    for (auto* i : bb.insts) print_instr(i);
}

void IRPrinter::print_instr(const Instruction* i) {
    os_ << "  ";
    switch (i->opcode) {
        case Opcode::ALLOCA:
            os_ << i->name << " = alloca";
            break;
        case Opcode::LOAD:
            os_ << i->name << " = load " << val_str(i->operands[0]);
            break;
        case Opcode::STORE:
            os_ << "store " << val_str(i->operands[0]) << ", " << val_str(i->operands[1]);
            break;
        case Opcode::ADD: case Opcode::SUB: case Opcode::MUL:
        case Opcode::DIV: case Opcode::MOD: case Opcode::AND: case Opcode::OR:
        case Opcode::CMP_EQ: case Opcode::CMP_NEQ: case Opcode::CMP_LT:
        case Opcode::CMP_GT: case Opcode::CMP_LEQ: case Opcode::CMP_GEQ:
            os_ << i->name << " = " << opcode_str(i->opcode)
                << " " << val_str(i->operands[0]) << ", " << val_str(i->operands[1]);
            break;
        case Opcode::NOT:
            os_ << i->name << " = not " << val_str(i->operands[0]);
            break;
        case Opcode::CALL:
            os_ << i->name << " = call @" << i->fn_name << "(";
            for (size_t j = 0; j < i->operands.size(); ++j)
                os_ << val_str(i->operands[j]) << (j + 1 < i->operands.size() ? ", " : "");
            os_ << ")";
            break;
        case Opcode::PRINT:
            os_ << "print " << val_str(i->operands[0]);
            break;
        case Opcode::JMP:
            os_ << "jmp " << i->bb1->label;
            break;
        case Opcode::BR:
            os_ << "br " << val_str(i->operands[0])
                << ", " << i->bb1->label << ", " << i->bb2->label;
            break;
        case Opcode::RET:
            os_ << (i->operands.empty() ? "ret void" : "ret " + val_str(i->operands[0]));
            break;
    }
    os_ << "\n";
}