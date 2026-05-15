#include <gtest/gtest.h>
#include "semantic.hpp"
#include "ast.hpp"

TEST(SemanticTest, UndefinedVariable) {
    SemanticAnalyzer sem;
    VariableNode var("undeclared_var");
    var.accept(sem);

    EXPECT_TRUE(sem.has_errors());
    EXPECT_EQ(sem.get_errors().size(), 1);
    EXPECT_NE(sem.get_errors()[0].find("Undefined variable"), std::string::npos);
}

TEST(SemanticTest, ValidVariableScope) {
    SemanticAnalyzer sem;
    auto assign = std::make_unique<AssignmentNode>("x", std::make_unique<NumberNode>(42));
    VariableNode var_use("x");

    assign->accept(sem);
    var_use.accept(sem);

    EXPECT_FALSE(sem.has_errors());
}

TEST(SemanticTest, BreakOutsideLoop) {
    SemanticAnalyzer sem;
    BreakNode brk;
    brk.accept(sem);

    EXPECT_TRUE(sem.has_errors());
    EXPECT_NE(sem.get_errors()[0].find("not within a loop"), std::string::npos);
}
