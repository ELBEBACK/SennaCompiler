#include "dce.hpp"

#include <algorithm>
#include <queue>
#include <unordered_map>
#include <unordered_set>


static bool is_pure(Opcode op) {
    switch (op) {
        case Opcode::ADD:
        case Opcode::SUB:
        case Opcode::MUL:
        case Opcode::DIV:
        case Opcode::MOD:
        case Opcode::AND:
        case Opcode::OR:
        case Opcode::NOT:
        case Opcode::CMP_EQ:
        case Opcode::CMP_NEQ:
        case Opcode::CMP_LT:
        case Opcode::CMP_GT:
        case Opcode::CMP_LEQ:
        case Opcode::CMP_GEQ:
        case Opcode::PHI:
        case Opcode::LOAD:
            return true;
        default:
            return false; 
    }
}


bool DCE::run(Function& fn) {

    std::unordered_map<Value*, uint32_t> use_count;

    for (const auto& bb : fn.blocks)
        for (auto* inst : bb->insts)
            use_count.emplace(inst, 0);         

    for (const auto& bb : fn.blocks)
        for (auto* inst : bb->insts)
            for (auto* op : inst->operands)
                if (use_count.count(op))        
                    ++use_count[op];


    std::queue<Instruction*>      worklist;
    std::unordered_set<Instruction*> dead;

    for (const auto& bb : fn.blocks)
        for (auto* inst : bb->insts)
            if (is_pure(inst->opcode) && use_count[inst] == 0)
                worklist.push(inst);

    
    while (!worklist.empty()) {
        auto* inst = worklist.front();
        worklist.pop();

        if (dead.count(inst)) continue;
        dead.insert(inst);

        for (auto* op : inst->operands) {
            auto it = use_count.find(op);
            if (it == use_count.end()) continue;    

            if (it->second > 0)
                --it->second;

            if (it->second == 0)
                if (auto* op_inst = dynamic_cast<Instruction*>(op))
                    if (is_pure(op_inst->opcode) && !dead.count(op_inst))
                        worklist.push(op_inst);
        }
    }

    if (dead.empty()) return false;

    for (auto& bb : fn.blocks) {
        auto& insts = bb->insts;
        insts.erase(
            std::remove_if(insts.begin(), insts.end(),
                [&](Instruction* i) { return dead.count(i) != 0; }),
            insts.end());
    }

    return true;
}
