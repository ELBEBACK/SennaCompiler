#include <gtest/gtest.h>
#include "ast.hpp"

TEST(ASTTest, BinaryOpAssociativity) {
    auto num1 = std::make_unique<NumberNode>(10);
    auto num2 = std::make_unique<NumberNode>(20);
    auto num3 = std::make_unique<NumberNode>(30);

    auto mul = std::make_unique<BinaryOpNode>(BinOp::MUL, std::move(num2), std::move(num3));
    auto add = std::make_unique<BinaryOpNode>(BinOp::ADD, std::move(num1), std::move(mul));

    EXPECT_EQ(add->op, BinOp::ADD);
    auto* right_child = dynamic_cast<BinaryOpNode*>(add->right.get());
    ASSERT_NE(right_child, nullptr);
    EXPECT_EQ(right_child->op, BinOp::MUL);
}

TEST(ASTTest, CompoundAssignment) {
    auto expr = std::make_unique<NumberNode>(5);
    CompoundAssignNode comp_assign("x", BinOp::ADD_ASSIGN, std::move(expr));

    EXPECT_EQ(comp_assign.var_name, "x");
    EXPECT_EQ(comp_assign.op, BinOp::ADD_ASSIGN);
}

TEST(ASTTest, FunctionDeclParams) {
    std::vector<Param> params = {{"a"}, {"b"}};
    auto body = std::make_unique<BlockNode>();
    FnDeclNode fn("my_func", params, std::move(body));

    EXPECT_EQ(fn.name, "my_func");
    EXPECT_EQ(fn.params.size(), 2);
    EXPECT_EQ(fn.params[0].name, "a");
}
