#pragma once
#include <unordered_set>

#include "ir.hpp"
#include "dom_front.hpp"

std::unordered_set<BasicBlock*> compute_phi_blocks(
    const DomFronts&                       fronts,
    const std::unordered_set<BasicBlock*>& def_sites);
