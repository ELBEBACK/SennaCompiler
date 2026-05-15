#pragma once
#include "ir.hpp"
#include "dom_tree.hpp"
#include <string>
#include <vector>
#include <unordered_map>

class SSAVerifier {
public:
    struct Error {
        std::string message;
        std::string function;
        std::string block;
    };

    bool verify(const Function& fn, const DomTree& dom);
    const std::vector<Error>& errors() const { return errors_; }

private:
    using DefMap = std::unordered_map<const Instruction*, const BasicBlock*>;

    std::vector<Error> errors_;
    std::string        cur_fn_;

    void check_phi(const BasicBlock& bb, const Instruction& phi,
                   const DomTree& dom, const DefMap& def_block);

    void check_dominance(const BasicBlock& bb, const Instruction& inst,
                         const DomTree& dom, const DefMap& def_block);

    void report(const std::string& msg, const std::string& bb_lbl = "");
};