#include "dom_front.hpp"
#include "explainer.hpp"

#include <string>

void DomFronts::build(const Function& fn, const DomTree& dom) {
    df.clear();

    auto& ex = Explainer::get();

    for (const auto& bb_owner : fn.blocks) {
        BasicBlock* bb = bb_owner.get();

        if (bb->preds.size() < 2) continue;

        if (ex.active())
            ex.log("fdom", fn.name,
                   bb->label + " has " +
                   std::to_string(bb->preds.size()) +
                   " predecessor(s) — DF walk triggered");

        for (BasicBlock* pred : bb->preds) {
            BasicBlock* cursor = pred;

            if (ex.active())
                ex.log("fdom", fn.name,
                       "  pred " + pred->label +
                       " — cursor starts at " + cursor->label);

            while (true) {
                auto idom_it = dom.idom.find(bb);

                if (idom_it != dom.idom.end() && cursor == idom_it->second) {
                    if (ex.active())
                        ex.log("fdom", fn.name,
                               "  cursor " + cursor->label +
                               " = idom(" + bb->label + ") — walk stops");
                    break;
                }

                df[cursor].insert(bb);

                if (ex.active()) {
                    std::string idom_label =
                        (idom_it != dom.idom.end())
                        ? idom_it->second->label
                        : "(none)";
                    ex.log("fdom", fn.name,
                           "  cursor " + cursor->label +
                           " \u2260 idom(" + bb->label + ")=" + idom_label +
                           " \u2192 DF[" + cursor->label + "] += " + bb->label);
                }

                auto cursor_idom = dom.idom.find(cursor);
                if (cursor_idom == dom.idom.end()) {
                    if (ex.active())
                        ex.log("fdom", fn.name,
                               "  " + cursor->label +
                               " has no idom — walk stops");
                    break;
                }

                std::string prev_label = cursor->label;
                cursor = cursor_idom->second;

                if (ex.active())
                    ex.log("fdom", fn.name,
                           "  walk up: " + prev_label + " \u2192 " + cursor->label);
            }
        }
    }
}
