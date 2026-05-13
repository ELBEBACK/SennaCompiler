#include "llvm_emit.hpp"

std::string LLVMEmitter::fresh() {
    return "%ll." + std::to_string(ll_cnt_++);
}

std::string LLVMEmitter::v(const Value* val) const {
    if (!val) return "undef";
    if (auto* c = dynamic_cast<const ConstantInt*>(val))
        return std::to_string(c->val);
    if (dynamic_cast<const UndefValue*>(val))
        return "undef";
    if (dynamic_cast<const IrParam*>(val))
        return "%" + val->name;
    return val->name;
}

std::string LLVMEmitter::lbl(const BasicBlock* bb) const {
    return "%" + bb->label;
}

void LLVMEmitter::emit_prelude() {
    os_ << "declare i32 @printf(i8* nocapture readonly, ...)\n";
    os_ << "declare i32 @scanf(i8* nocapture, ...)\n";
    os_ << "\n";

    os_ << "@.fmt_print = private unnamed_addr constant [6 x i8] "
           "c\"%lld\\0A\\00\", align 1\n";
    os_ << "@.fmt_read  = private unnamed_addr constant [5 x i8] "
           "c\"%lld\\00\", align 1\n";
    os_ << "\n";
}

void LLVMEmitter::emit_fn(const Function& fn) {
    const bool is_main = (fn.name == "main");

    if (is_main) {
        os_ << "define i32 @main()";
    } else {
        os_ << "define i64 @" << fn.name << "(";
        for (size_t i = 0; i < fn.params.size(); ++i) {
            os_ << "i64 %" << fn.params[i]->name;
            if (i + 1 < fn.params.size()) os_ << ", ";
        }
        os_ << ")";
    }

    os_ << " {\n";

    for (const auto& bb : fn.blocks)
        emit_bb(*bb, is_main);

    os_ << "}\n\n";
}

void LLVMEmitter::emit_bb(const BasicBlock& bb, bool is_main) {
    os_ << bb.label << ":\n";
    for (const auto* inst : bb.insts)
        emit_instr(inst, is_main);
}

void LLVMEmitter::emit_instr(const Instruction* inst, bool is_main) {
    os_ << "  ";

    switch (inst->opcode) {
        case Opcode::ALLOCA:
            os_ << inst->name << " = alloca i64, align 8\n";
            break;

        case Opcode::LOAD:
            os_ << inst->name << " = load i64, i64* " << v(inst->operands[0])
                << ", align 8\n";
            break;

        case Opcode::STORE:
            os_ << "store i64 " << v(inst->operands[0])
                << ", i64* " << v(inst->operands[1]) << ", align 8\n";
            break;

        case Opcode::ADD:
            os_ << inst->name << " = add  i64 " << v(inst->operands[0])
                << ", " << v(inst->operands[1]) << "\n";
            break;
        case Opcode::SUB:
            os_ << inst->name << " = sub  i64 " << v(inst->operands[0])
                << ", " << v(inst->operands[1]) << "\n";
            break;
        case Opcode::MUL:
            os_ << inst->name << " = mul  i64 " << v(inst->operands[0])
                << ", " << v(inst->operands[1]) << "\n";
            break;
        case Opcode::DIV:
            os_ << inst->name << " = sdiv i64 " << v(inst->operands[0])
                << ", " << v(inst->operands[1]) << "\n";
            break;
        case Opcode::MOD:
            os_ << inst->name << " = srem i64 " << v(inst->operands[0])
                << ", " << v(inst->operands[1]) << "\n";
            break;

        #define EMIT_ICMP(pred)                                                    \
            {                                                                      \
                std::string tmp = fresh();                                         \
                os_ << tmp << " = icmp " pred " i64 "                             \
                    << v(inst->operands[0]) << ", " << v(inst->operands[1]) << "\n"; \
                os_ << "  " << inst->name << " = zext i1 " << tmp << " to i64\n"; \
            }

        case Opcode::CMP_EQ:  EMIT_ICMP("eq")  break;
        case Opcode::CMP_NEQ: EMIT_ICMP("ne")  break;
        case Opcode::CMP_LT:  EMIT_ICMP("slt") break;
        case Opcode::CMP_GT:  EMIT_ICMP("sgt") break;
        case Opcode::CMP_LEQ: EMIT_ICMP("sle") break;
        case Opcode::CMP_GEQ: EMIT_ICMP("sge") break;
        #undef EMIT_ICMP

        case Opcode::AND: {
            std::string a1 = fresh(), b1 = fresh(), r1 = fresh();
            os_ << a1 << " = icmp ne i64 " << v(inst->operands[0]) << ", 0\n";
            os_ << "  " << b1 << " = icmp ne i64 " << v(inst->operands[1]) << ", 0\n";
            os_ << "  " << r1 << " = and i1 " << a1 << ", " << b1 << "\n";
            os_ << "  " << inst->name << " = zext i1 " << r1 << " to i64\n";
            break;
        }
        case Opcode::OR: {
            std::string a1 = fresh(), b1 = fresh(), r1 = fresh();
            os_ << a1 << " = icmp ne i64 " << v(inst->operands[0]) << ", 0\n";
            os_ << "  " << b1 << " = icmp ne i64 " << v(inst->operands[1]) << ", 0\n";
            os_ << "  " << r1 << " = or  i1 " << a1 << ", " << b1 << "\n";
            os_ << "  " << inst->name << " = zext i1 " << r1 << " to i64\n";
            break;
        }

        case Opcode::NOT: {
            std::string tmp = fresh();
            os_ << tmp << " = icmp eq i64 " << v(inst->operands[0]) << ", 0\n";
            os_ << "  " << inst->name << " = zext i1 " << tmp << " to i64\n";
            break;
        }

        case Opcode::PHI:
            os_ << inst->name << " = phi i64 ";
            for (uint32_t j = 0; j < inst->operands.size(); ++j) {
                os_ << "[ " << v(inst->operands[j])
                    << ", " << lbl(inst->phi_preds[j]) << " ]";
                if (j + 1 < inst->operands.size()) os_ << ", ";
            }
            os_ << "\n";
            break;

        case Opcode::CALL:
            os_ << inst->name << " = call i64 @" << inst->fn_name << "(";
            for (size_t j = 0; j < inst->operands.size(); ++j) {
                os_ << "i64 " << v(inst->operands[j]);
                if (j + 1 < inst->operands.size()) os_ << ", ";
            }
            os_ << ")\n";
            break;

        case Opcode::PRINT:
            os_ << "call i32 (i8*, ...) @printf("
                "i8* getelementptr inbounds ([6 x i8], [6 x i8]* @.fmt_print, "
                "i64 0, i64 0), i64 " << v(inst->operands[0]) << ")\n";
            break;

        case Opcode::READ: {
            std::string slot = fresh();
            os_ << slot << " = alloca i64, align 8\n";
            os_ << "  call i32 (i8*, ...) @scanf("
                "i8* getelementptr inbounds ([5 x i8], [5 x i8]* @.fmt_read, "
                "i64 0, i64 0), i64* " << slot << ")\n";
            os_ << "  " << inst->name << " = load i64, i64* " << slot
                << ", align 8\n";
            break;
        }

        case Opcode::JMP:
            os_ << "br label " << lbl(inst->bb1) << "\n";
            break;

        case Opcode::BR: {
            std::string cond1 = fresh();
            os_ << cond1 << " = icmp ne i64 " << v(inst->operands[0]) << ", 0\n";
            os_ << "  br i1 " << cond1
                << ", label " << lbl(inst->bb1)
                << ", label " << lbl(inst->bb2) << "\n";
            break;
        }

        case Opcode::RET:
            if (is_main) {
                if (inst->operands.empty()) {
                    os_ << "ret i32 0\n";
                } else {
                    std::string tmp = fresh();
                    os_ << tmp << " = trunc i64 " << v(inst->operands[0]) << " to i32\n";
                    os_ << "  ret i32 " << tmp << "\n";
                }
            } else {
                if (inst->operands.empty())
                    os_ << "ret i64 0\n";
                else
                    os_ << "ret i64 " << v(inst->operands[0]) << "\n";
            }
            break;
    }
}

void LLVMEmitter::emit(const Module& mod) {
    os_ << "; SennaC → LLVM IR  (target: x86-64 Linux, clang -O0)\n";
    os_ << "target triple = \"x86_64-pc-linux-gnu\"\n\n";

    emit_prelude();

    for (const auto& fn : mod.functions)
        emit_fn(*fn);
}
