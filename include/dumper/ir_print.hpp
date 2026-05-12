#pragma once
#include "ir.hpp"
#include <ostream>

class IRPrinter {
public:
    explicit IRPrinter(std::ostream& os) : os_(os) {}
    void print(const Module& mod);

private:
    std::ostream& os_;

    void print_fn(const Function& fn);
    void print_bb(const BasicBlock& bb);
    void print_instr(const Instruction* i);
};