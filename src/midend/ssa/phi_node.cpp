#include "phi_node.hpp"
#include <vector>

std::unordered_set<BasicBlock*> compute_phi_blocks(
    const DomFronts&                       fronts,
    const std::unordered_set<BasicBlock*>& def_sites)
{
    std::unordered_set<BasicBlock*> phi_blocks;
    std::unordered_set<BasicBlock*> ever_in_worklist = def_sites;
    std::vector<BasicBlock*>        worklist(def_sites.begin(), def_sites.end());

    while (!worklist.empty()) {
        BasicBlock* x = worklist.back();
        worklist.pop_back();

        auto fd_it = fronts.df.find(x);
        if (fd_it == fronts.df.end()) continue;

        for (BasicBlock* y : fd_it->second) {
            if (phi_blocks.insert(y).second)
                if (ever_in_worklist.insert(y).second)
                    worklist.push_back(y);
        }
    }

    return phi_blocks;
}
