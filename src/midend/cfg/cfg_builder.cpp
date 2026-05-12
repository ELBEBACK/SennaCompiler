#include "cfg_builder.hpp"

void CFGBuilder::build(Module& mod) {
    for (auto& fn : mod.functions) {
        build_function(*fn);
    }
}

void CFGBuilder::build_function(Function& fn) {

    for (auto& bb : fn.blocks) {
        bb->succs.clear();
        bb->preds.clear();
    }

    for (auto& bb : fn.blocks) {
        build_block_edges(*bb);
    }
}

void CFGBuilder::build_block_edges(BasicBlock& bb) {
    if (bb.insts.empty()) return;

    Instruction* last = bb.insts.back();

    switch (last->opcode) {
        case Opcode::JMP:
            if (last->bb1) {
                bb.add_edge(last->bb1);
            }
            break;

        case Opcode::BR:
            if (last->bb1) {
                bb.add_edge(last->bb1);
            }
            if (last->bb2) {
                bb.add_edge(last->bb2);
            }
            break;

        case Opcode::RET:
            break;

        default:
            break;
    }
}
