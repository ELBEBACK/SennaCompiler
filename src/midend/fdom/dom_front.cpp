#include "dom_front.hpp"


void DomFronts::build(const Function& fn, const DomTree& dom) {
    df.clear();

    for (const auto& bb_owner : fn.blocks) {
        BasicBlock* bb = bb_owner.get();

        if (bb->preds.size() < 2) continue;

        for (BasicBlock* pred : bb->preds) {
            BasicBlock* cursor = pred;

            while (true) {
                auto idom_it = dom.idom.find(bb);
                if (idom_it != dom.idom.end() && cursor == idom_it->second) break;

                df[cursor].insert(bb);

                auto cursor_idom = dom.idom.find(cursor);
                if (cursor_idom == dom.idom.end()) break;   
                cursor = cursor_idom->second;
            }
        }
    }
}