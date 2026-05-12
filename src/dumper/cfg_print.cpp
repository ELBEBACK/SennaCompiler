#include "cfg_print.hpp"

static std::string opcode_str(Opcode op) {
    switch (op) {
        case Opcode::ADD:     return "add";     case Opcode::SUB:     return "sub";
        case Opcode::MUL:     return "mul";     case Opcode::DIV:     return "div";
        case Opcode::MOD:     return "mod";     case Opcode::AND:     return "and";
        case Opcode::OR:      return "or";      case Opcode::NOT:     return "not";
        case Opcode::CMP_EQ:  return "cmp.eq";  case Opcode::CMP_NEQ: return "cmp.neq";
        case Opcode::CMP_LT:  return "cmp.lt";  case Opcode::CMP_GT:  return "cmp.gt";
        case Opcode::CMP_LEQ: return "cmp.leq"; case Opcode::CMP_GEQ: return "cmp.geq";
        case Opcode::BR:      return "br";      case Opcode::JMP:     return "jmp";
        case Opcode::RET:     return "ret";     case Opcode::STORE:   return "store";
        case Opcode::LOAD:    return "load";    case Opcode::CALL:    return "call";
        case Opcode::PRINT:   return "print";   case Opcode::ALLOCA:  return "alloca";
        default:              return "?";
    }
}

static std::string val_str(const Value* v) {
    if (!v) return "<null>";
    if (auto* c = dynamic_cast<const ConstantInt*>(v))
        return std::to_string(c->val);
    return v->name;
}

void CFGPrinter::print(const Module& mod) {
    for (auto& fn : mod.functions) {
        print_fn(*fn);
    }
}

void CFGPrinter::print_fn(const Function& fn) {
    os_ << "digraph \"CFG_" << fn.name << "\" {\n";
    os_ << "  node [shape=record, fontname=\"Courier\", fontsize=10];\n";

    for (auto& bb : fn.blocks) {
        print_bb(*bb);
    }

    for (auto& bb : fn.blocks) {
        for (auto* succ : bb->succs) {
            os_ << "  \"" << bb->label << "\" -> \"" << succ->label << "\";\n";
        }
    }

    os_ << "}\n";
}

void CFGPrinter::print_bb(const BasicBlock& bb) {
    os_ << "  \"" << bb.label << "\" [label=\"{" << bb.label << ":|";
    for (auto* i : bb.insts) {
        os_ <<escape(format_instr(i)) << "\\l";
    }
    os_ << "}\"];\n";
}

std::string CFGPrinter::format_instr(const Instruction* i) {
    std::string res;
    switch (i->opcode) {
        case Opcode::ALLOCA:
            res += i->name + " = alloca"; break;
        case Opcode::LOAD:
            res += i->name + " = load " + val_str(i->operands[0]); break;
        case Opcode::STORE:
            res += "store " + val_str(i->operands[0]) + ", " + val_str(i->operands[1]); break;
        case Opcode::BR:
            res += "br " + val_str(i->operands[0]) + ", " + i->bb1->label + ", " + i->bb2->label; break;
        case Opcode::JMP:
            res += "jmp " + i->bb1->label; break;
        case Opcode::RET:
            res += (i->operands.empty() ? "ret void" : "ret " + val_str(i->operands[0])); break;
        case Opcode::CALL:
            if (!i->name.empty()) res += i->name + " = ";
            res += "call @" + i->fn_name + "(";
            for (size_t j = 0; j < i->operands.size(); ++j)
                res += val_str(i->operands[j]) + (j + 1 < i->operands.size() ? ", " : "");
            res += ")";
            break;
        default:
            if (!i->name.empty()) res += i->name + " = ";
            res += opcode_str(i->opcode);
            for (auto* op : i->operands) res += " " + val_str(op);
            break;
    }
    return res;
}

std::string CFGPrinter::escape(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '<' || c == '>' || c == '{' || c == '}' || c == '|') continue;
        out += c;
    }
    return out;
}
