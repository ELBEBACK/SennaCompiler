#pragma once
#include <unordered_map>
#include <unordered_set>

#include "ir.hpp"
#include "dom_tree.hpp"


class DomFronts {
public:
    std::unordered_map<BasicBlock*, std::unordered_set<BasicBlock*>> df;

    void build(const Function& fn, const DomTree& dom);
};
