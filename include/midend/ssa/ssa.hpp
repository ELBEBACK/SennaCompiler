#pragma once
#include "ir.hpp"
#include "dom_tree.hpp"
#include "dom_front.hpp"

class Mem2Reg {
public:
    bool run(Function& fn, const DomTree& dom, const DomFronts& fronts);
};
