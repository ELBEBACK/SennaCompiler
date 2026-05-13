#pragma once
#include "ir.hpp"
#include <string>
#include <vector>

class IRValidator {
public:
    struct ValidationError {
        std::string message;
        std::string function_name;
        std::string block_label;
    };

    bool validate(const Module& mod);
    const std::vector<ValidationError>& get_errors() const { return errors_; }

private:
    std::vector<ValidationError> errors_;

    void verify_function(const Function& fn);
    void verify_block(const Function& fn, const BasicBlock& bb);
    void verify_instruction(const Function& fn, const BasicBlock& bb, const Instruction& inst);

    void check_type_invariants(const Function& fn, const BasicBlock& bb, const Instruction& inst);
    void check_use_def(const Function& fn, const Instruction& inst);

    void report(const std::string& msg, const Function& fn, const std::string& bb_lbl = "");
};
