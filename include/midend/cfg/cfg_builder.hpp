#pragma once
#include "ir.hpp"

class CFGBuilder {
public:
    void build(Module& mod);

private:
    void build_function(Function& fn);
    void build_block_edges(BasicBlock& bb);
};
