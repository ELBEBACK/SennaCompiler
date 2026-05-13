#pragma once
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "ir.hpp"


class DomTree {
public:
    std::unordered_map<BasicBlock*, BasicBlock*>              idom;
    std::unordered_map<BasicBlock*, std::vector<BasicBlock*>> children;
    std::unordered_map<BasicBlock*, std::unordered_set<BasicBlock*>> dom_sets;

    void build(const Function& fn);

    bool dominates(BasicBlock* a, BasicBlock* b) const;

    bool strictly_dominates(BasicBlock* a, BasicBlock* b) const;
};
