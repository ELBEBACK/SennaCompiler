#pragma once
#include "ir.hpp"
#include <ostream>
#include <string>

class CFGPrinter {
public:
    explicit CFGPrinter(std::ostream& os) : os_(os) {}

    void print(const Module& mod);
    void print_fn(const Function& fn);

private:
    std::ostream& os_;

    void print_bb(const BasicBlock& bb);
    std::string format_instr(const Instruction* i);

    std::string escape(const std::string& s);
};
