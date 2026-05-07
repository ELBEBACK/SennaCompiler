#include "semantic.hpp"

SemanticAnalyzer::SemanticAnalyzer() {
    enter_scope();
}

void SemanticAnalyzer::enter_scope() {
    scopes.push_back({});
}

void SemanticAnalyzer::exit_scope() {
    scopes.pop_back();
}

void SemanticAnalyzer::define_symbol(const std::string& name, bool is_function) {
    scopes.back()[name] = {is_function};
}

bool SemanticAnalyzer::resolve_symbol(const std::string& name) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        if (it->find(name) != it->end()) {
            return true;
        }
    }
    return false;
}

void SemanticAnalyzer::report_error(const std::string& message) {
    std::cerr << "[-] Semantic Error: " << message << std::endl;
    has_error = true;
}


void SemanticAnalyzer::visit(NumberNode& node) {}

void SemanticAnalyzer::visit(VariableNode& node) {
    if (!resolve_symbol(node.name)) {
        report_error("Undefined variable '" + node.name + "'");
    }
}

void SemanticAnalyzer::visit(AssignmentNode& node) {
    node.expr->accept(*this);

    if (!resolve_symbol(node.var_name)) {
        define_symbol(node.var_name, false);
    }
}

void SemanticAnalyzer::visit(BinaryOpNode& node) {
    node.left->accept(*this);
    node.right->accept(*this);
}

void SemanticAnalyzer::visit(UnaryOpNode& node) {
    node.operand->accept(*this);
}

void SemanticAnalyzer::visit(PrintNode& node) {
    node.expr->accept(*this);
}

void SemanticAnalyzer::visit(BlockNode& node) {
    enter_scope();
    for (auto& stmt : node.statements) {
        stmt->accept(*this);
    }
    exit_scope();
}

void SemanticAnalyzer::visit(IfStmtNode& node) {
    node.condition->accept(*this);
    node.true_block->accept(*this);
    if (node.false_block) {
        node.false_block->accept(*this);
    }
}

void SemanticAnalyzer::visit(WhileStmtNode& node) {
    node.condition->accept(*this);

    loop_depth++;
    node.body->accept(*this);
    loop_depth--;
}

void SemanticAnalyzer::visit(ForNode& node) {
    enter_scope();

    node.init->accept(*this);
    node.cond->accept(*this);
    node.step->accept(*this);

    loop_depth++;
    node.body->accept(*this);
    loop_depth--;

    exit_scope();
}

void SemanticAnalyzer::visit(FnDeclNode& node) {
    if (scopes.back().count(node.name)) {
        report_error("Function '" + node.name + "' is already defined in this scope.");
    } else {
        define_symbol(node.name, true);
    }

    enter_scope();
    for (const auto& param : node.params) {
        if (scopes.back().count(param.name)) {
            report_error("Duplicate parameter '" + param.name + "' in function '" + node.name + "'");
        }
        define_symbol(param.name, false);
    }

    node.body->accept(*this);
    exit_scope();
}

void SemanticAnalyzer::visit(CallExprNode& node) {
    if (!resolve_symbol(node.function_name)) {
        report_error("Call to undefined function '" + node.function_name + "'");
    }

    for (auto& arg : node.arguments) {
        arg->accept(*this);
    }
}

void SemanticAnalyzer::visit(ReturnStmtNode& node) {
    if (node.expr) {
        node.expr->accept(*this);
    }
}

void SemanticAnalyzer::visit(BreakNode& node) {
    if(loop_depth == 0) {
        report_error("break not within a loop");
    }
}
void SemanticAnalyzer::visit(ContinueNode& node) {
    if(loop_depth == 0) {
        report_error("continue not within");
    }
}
