#include <gtest/gtest.h>
#include "dom_tree.hpp"
#include "ir.hpp"

TEST(DomTreeTest, LinearDominance) {
    Function fn;
    BasicBlock* b1 = fn.new_block("1");
    BasicBlock* b2 = fn.new_block("2");
    BasicBlock* b3 = fn.new_block("3");

    fn.entry = b1;
    b1->add_edge(b2);
    b2->add_edge(b3);

    DomTree dom;
    dom.build(fn);

    EXPECT_TRUE(dom.dominates(b1, b2));
    EXPECT_TRUE(dom.dominates(b2, b3));
    EXPECT_TRUE(dom.strictly_dominates(b1, b3));
    EXPECT_FALSE(dom.dominates(b3, b1));
}
