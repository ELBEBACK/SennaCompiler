#include <gtest/gtest.h>
#include "ir.hpp"

TEST(IRTest, ControlFlowGraphConnections) {
    Function fn;
    BasicBlock* b1 = fn.new_block("b1");
    BasicBlock* b2 = fn.new_block("b2");

    b1->add_edge(b2);

    EXPECT_EQ(b1->succs.size(), 1);
    EXPECT_EQ(b1->succs[0], b2);
    EXPECT_EQ(b2->preds.size(), 1);
    EXPECT_EQ(b2->preds[0], b1);
}

TEST(IRTest, PruneDeadBlocks) {
    Function fn;
    fn.entry = fn.new_block("entry");
    fn.new_block("dead_block");

    fn.prune_dead_blocks();
    EXPECT_EQ(fn.blocks.size(), 1);
    EXPECT_EQ(fn.blocks[0]->label, "entry");
}
