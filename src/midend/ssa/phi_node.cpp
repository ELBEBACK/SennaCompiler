#include "phi_node.hpp"
#include "explainer.hpp"

#include <algorithm>
#include <string>
#include <vector>

static std::string fmt_bb_set(const std::unordered_set<BasicBlock*>& s) {
    std::vector<std::string> labels;
    labels.reserve(s.size());
    for (auto* b : s) labels.push_back(b->label);
    std::sort(labels.begin(), labels.end());

    std::string out = "{";
    for (size_t i = 0; i < labels.size(); ++i) {
        if (i) out += ", ";
        out += labels[i];
    }
    out += "}";
    return out;
}

std::unordered_set<BasicBlock*> compute_phi_blocks(
    const DomFronts&                       fronts,
    const std::unordered_set<BasicBlock*>& def_sites,
    std::string_view                       fn_name,
    std::string_view                       var_name)
{
    auto& ex = Explainer::get();

    if (ex.active()) {
        ex.log("phi", fn_name,
               std::string(var_name) +
               " — def sites: " + fmt_bb_set(def_sites));
        ex.log("phi", fn_name,
               std::string(var_name) +
               " — worklist init: " + fmt_bb_set(def_sites));
    }

    std::unordered_set<BasicBlock*> phi_blocks;
    std::unordered_set<BasicBlock*> ever_in_worklist = def_sites;
    std::vector<BasicBlock*>        worklist(def_sites.begin(), def_sites.end());

    while (!worklist.empty()) {
        BasicBlock* x = worklist.back();
        worklist.pop_back();

        auto fd_it = fronts.df.find(x);

        if (fd_it == fronts.df.end() || fd_it->second.empty()) {
            if (ex.active())
                ex.log("phi", fn_name,
                       std::string(var_name) +
                       " — pop " + x->label + " \u2192 DF = {} (no placements)");
            continue;
        }

        if (ex.active())
            ex.log("phi", fn_name,
                   std::string(var_name) +
                   " — pop " + x->label +
                   " \u2192 DF = " + fmt_bb_set(fd_it->second));

        for (BasicBlock* y : fd_it->second) {
            if (phi_blocks.insert(y).second) {
                if (ever_in_worklist.insert(y).second) {
                    worklist.push_back(y);
                    if (ex.active())
                        ex.log("phi", fn_name,
                               std::string(var_name) +
                               " —   " + y->label +
                               ": phi placed, enters worklist");
                } else {
                    if (ex.active())
                        ex.log("phi", fn_name,
                               std::string(var_name) +
                               " —   " + y->label +
                               ": phi placed (already visited, not re-queued)");
                }
            } else {
                if (ex.active())
                    ex.log("phi", fn_name,
                           std::string(var_name) +
                           " —   " + y->label +
                           ": phi already placed — skip");
            }
        }
    }

    if (ex.active())
        ex.log("phi", fn_name,
               std::string(var_name) +
               " — placement complete: " + fmt_bb_set(phi_blocks));

    return phi_blocks;
}
