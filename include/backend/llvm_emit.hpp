#pragma once
#include "ir.hpp"
#include <ostream>
#include <string>

class LLVMEmitter {
public:
    explicit LLVMEmitter(std::ostream& os) : os_(os) {}
    void emit(const Module& mod);

private:
    std::ostream& os_;
    uint32_t      ll_cnt_ = 0;

    std::string fresh();

    void emit_prelude();
    void emit_fn(const Function& fn);
    void emit_bb(const BasicBlock& bb, bool is_main);
    void emit_instr(const Instruction* inst, bool is_main);

    std::string v(const Value* val) const;

    std::string lbl(const BasicBlock* bb) const;
};
