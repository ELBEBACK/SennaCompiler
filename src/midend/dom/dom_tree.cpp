#include "dom_tree.hpp"


void DomTree::build(const Function& fn) {
    idom.clear();
    children.clear();
    dom_sets.clear();

    BasicBlock* entry = fn.entry;

    std::unordered_set<BasicBlock*> all_blocks;
    for (const auto& bb : fn.blocks)
        all_blocks.insert(bb.get());


    dom_sets[entry] = {entry};
    for (BasicBlock* bb : all_blocks)
        if (bb != entry)
            dom_sets[bb] = all_blocks;

    bool sets_changed = true;
    while (sets_changed) {
        sets_changed = false;

        for (BasicBlock* bb : all_blocks) {
            if (bb == entry)   continue;
            if (bb->preds.empty()) continue;

            std::unordered_set<BasicBlock*> new_dom = dom_sets[bb->preds[0]];

            for (uint32_t i = 1; i < bb->preds.size(); ++i) {
                const auto& pred_dom = dom_sets[bb->preds[i]];
                for (auto it = new_dom.begin(); it != new_dom.end(); ) {
                    if (pred_dom.count(*it) == 0)
                        it = new_dom.erase(it);
                    else
                        ++it;
                }
            }

            new_dom.insert(bb);

            if (new_dom != dom_sets[bb]) {
                dom_sets[bb] = std::move(new_dom);
                sets_changed  = true;
            }
        }
    }


    for (BasicBlock* bb : all_blocks) {
        if (bb == entry) continue;

        std::unordered_set<BasicBlock*> sdom = dom_sets[bb];
        sdom.erase(bb);

        for (BasicBlock* cand : sdom) {
            bool is_idom = true;
            for (BasicBlock* other : sdom) {
                if (other == cand) continue;
                if (dom_sets[other].count(cand)) {
                    is_idom = false;
                    break;
                }
            }
            if (is_idom) {
                idom[bb] = cand;
                children[cand].push_back(bb);
                break;
            }
        }
    }
}

bool DomTree::dominates(BasicBlock* a, BasicBlock* b) const {
    auto it = dom_sets.find(b);
    if (it == dom_sets.end()) return false;
    return it->second.count(a) != 0;
}

bool DomTree::strictly_dominates(BasicBlock* a, BasicBlock* b) const {
    return a != b && dominates(a, b);
}
