#include "ssa_verifier.hpp"
#include <algorithm>

bool SSAVerifier::verify(const Function& fn, const DomTree& dom) {
    errors_.clear();
    cur_fn_ = fn.name;

    DefMap def_block;
    for (const auto& bb : fn.blocks)
        for (const auto* inst : bb->insts)
            def_block[inst] = bb.get();

    for (const auto& bb : fn.blocks)
        for (const auto* inst : bb->insts)
            if (inst->opcode == Opcode::PHI)
                check_phi(*bb, *inst, dom, def_block);
            else
                check_dominance(*bb, *inst, dom, def_block);

    return errors_.empty();
}

void SSAVerifier::check_phi(const BasicBlock& bb, const Instruction& phi,
                             const DomTree& dom, const DefMap& def_block)
{
    if (phi.operands.size() != phi.phi_preds.size()) {
        report("PHI '" + phi.name + "' has " +
               std::to_string(phi.operands.size()) + " operands but " +
               std::to_string(phi.phi_preds.size()) + " predecessors",
               bb.label);
        return;
    }

    for (size_t i = 0; i < phi.phi_preds.size(); ++i) {
        BasicBlock* pred = phi.phi_preds[i];

        if (std::find(bb.preds.begin(), bb.preds.end(), pred) == bb.preds.end()) {
            report("PHI '" + phi.name + "' lists '" + pred->label +
                   "' as predecessor but it is not in preds of '" + bb.label + "'",
                   bb.label);
        }

        auto* op_inst = dynamic_cast<const Instruction*>(phi.operands[i]);
        if (!op_inst) continue;

        auto it = def_block.find(op_inst);
        if (it == def_block.end()) continue;

        if (!dom.dominates(const_cast<BasicBlock*>(it->second), pred)) {
            report("PHI '" + phi.name + "': operand '" + op_inst->name +
                   "' (in '" + it->second->label +
                   "') does not dominate predecessor '" + pred->label + "'",
                   bb.label);
        }
    }
}

void SSAVerifier::check_dominance(const BasicBlock& bb, const Instruction& inst,
                                   const DomTree& dom, const DefMap& def_block)
{
    for (const auto* op : inst.operands) {
        auto* op_inst = dynamic_cast<const Instruction*>(op);
        if (!op_inst) continue;

        auto it = def_block.find(op_inst);
        if (it == def_block.end()) continue;

        auto* def_bb = const_cast<BasicBlock*>(it->second);
        auto* use_bb = const_cast<BasicBlock*>(&bb);

        if (!dom.dominates(def_bb, use_bb)) {
            report("'" + op_inst->name + "' defined in '" + def_bb->label +
                   "' does not dominate use in '" + bb.label + "'",
                   bb.label);
        }
    }
}

void SSAVerifier::report(const std::string& msg, const std::string& bb_lbl) {
    errors_.push_back({msg, cur_fn_, bb_lbl});
}