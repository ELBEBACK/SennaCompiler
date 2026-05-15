#include "ssa.hpp"
#include "phi_node.hpp"

#include <algorithm>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>

static bool is_promotable(const Function& fn, const Instruction* alloca) {
    for (const auto& bb : fn.blocks) {
        for (const auto* inst : bb->insts) {
            for (const auto* op : inst->operands) {
                if (op == alloca
                    && inst->opcode != Opcode::LOAD
                    && inst->opcode != Opcode::STORE)
                    return false;
            }
        }
    }
    return true;
}

struct Renamer {
    Function&                                              fn;
    const DomTree&                                         dom;
    const std::unordered_set<Instruction*>&                promoted;
    const std::unordered_map<Instruction*, Instruction*>&  phi_to_alloca;
    std::unordered_map<Instruction*, std::stack<Value*>>   stacks;

    void rename(BasicBlock* bb) {
        std::unordered_map<Instruction*, uint32_t> push_counts;

        for (auto* inst : bb->insts) {
            if (inst->opcode != Opcode::PHI) break;
            auto it = phi_to_alloca.find(inst);
            if (it == phi_to_alloca.end()) continue;
            stacks[it->second].push(inst);
            push_counts[it->second]++;
        }

        std::unordered_map<Instruction*, Value*> load_repls;
        std::vector<Instruction*>                to_remove;

        for (auto* inst : bb->insts) {
            if (inst->opcode == Opcode::PHI) continue;

            for (auto*& op : inst->operands) {
                if (auto* as_inst = dynamic_cast<Instruction*>(op)) {
                    auto rep_it = load_repls.find(as_inst);
                    if (rep_it != load_repls.end())
                        op = rep_it->second;
                }
            }

            if (inst->opcode == Opcode::LOAD) {
                auto* src = dynamic_cast<Instruction*>(inst->operands[0]);
                if (src && promoted.count(src)) {
                    Value* cur = stacks[src].empty() ? fn.get_undef() : stacks[src].top();
                    load_repls[inst] = cur;
                    to_remove.push_back(inst);
                }

            } else if (inst->opcode == Opcode::STORE) {
                auto* dst = dynamic_cast<Instruction*>(inst->operands[1]);
                if (dst && promoted.count(dst)) {
                    stacks[dst].push(inst->operands[0]);
                    push_counts[dst]++;
                    to_remove.push_back(inst);
                }
            }
        }

        for (auto* succ : bb->succs) {
            for (auto* inst : succ->insts) {
                if (inst->opcode != Opcode::PHI) break;
                auto it = phi_to_alloca.find(inst);
                if (it == phi_to_alloca.end()) continue;
                Value* val = stacks[it->second].empty()
                           ? fn.get_undef()
                           : stacks[it->second].top();
                inst->operands.push_back(val);
                inst->phi_preds.push_back(bb);
            }
        }

        if (!to_remove.empty()) {
            std::unordered_set<Instruction*> remove_set(to_remove.begin(), to_remove.end());
            auto& insts = bb->insts;
            insts.erase(
                std::remove_if(insts.begin(), insts.end(),
                    [&](Instruction* i) { return remove_set.count(i); }),
                insts.end());
        }

        auto ch_it = dom.children.find(bb);
        if (ch_it != dom.children.end())
            for (auto* child : ch_it->second)
                rename(child);

        for (auto& [alloca, count] : push_counts)
            for (uint32_t i = 0; i < count; ++i)
                stacks[alloca].pop();
    }
};

bool Mem2Reg::run(Function& fn, const DomTree& dom, const DomFronts& fronts) {

    std::unordered_set<Instruction*> promotable;

    for (const auto& bb : fn.blocks)
        for (auto* inst : bb->insts)
            if (inst->opcode == Opcode::ALLOCA && is_promotable(fn, inst))
                promotable.insert(inst);

    if (promotable.empty()) return false;

    std::unordered_map<Instruction*, Instruction*> phi_to_alloca;

    for (auto* alloca : promotable) {
        std::unordered_set<BasicBlock*> def_sites;
        for (const auto& bb : fn.blocks)
            for (auto* inst : bb->insts)
                if (inst->opcode   == Opcode::STORE
                    && inst->operands.size() == 2
                    && inst->operands[1] == alloca)
                    def_sites.insert(bb.get());

        if (def_sites.empty()) continue;

        auto placement = compute_phi_blocks(fronts, def_sites,
                                            fn.name, alloca->name);

        for (auto* block : placement) {
            auto* phi = fn.make_inst(fn.new_temp(), Opcode::PHI, {});
            phi_to_alloca[phi] = alloca;
            block->insts.insert(block->insts.begin(), phi);
        }
    }

    Renamer renamer{fn, dom, promotable, phi_to_alloca};
    renamer.rename(fn.entry);

    for (auto& bb : fn.blocks) {
        auto& insts = bb->insts;
        insts.erase(
            std::remove_if(insts.begin(), insts.end(),
                [&](Instruction* i) {
                    return i->opcode == Opcode::ALLOCA && promotable.count(i);
                }),
            insts.end());
    }

    return true;
}
