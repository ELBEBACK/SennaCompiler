#include <gtest/gtest.h>
#include "ir.hpp"
#include "const_fold_prop.hpp"
#include "dce.hpp"
#include "exp_simplifier.hpp"

TEST(OptsTest, AlgebraicSimplification) {
    Function fn;
    BasicBlock* bb = fn.new_block("bb");
    Value* v = fn.add_param("x");
    Value* zero = fn.get_const(0);

    Instruction* add = fn.emit(bb, Opcode::ADD, {v, zero});

    AlgebraicSimplify as;
    EXPECT_TRUE(as.run(fn));

    EXPECT_EQ(add->operands[0], v);
}

TEST(OptsTest, DeadCodeElimination) {
    Function fn;
    BasicBlock* bb = fn.new_block("bb");
    Value* c1 = fn.get_const(10);

    Instruction* dead = fn.emit(bb, Opcode::ADD, {c1, c1});

    DCE dce;
    EXPECT_TRUE(dce.run(fn));
    EXPECT_EQ(bb->insts.size(), 0);
}
