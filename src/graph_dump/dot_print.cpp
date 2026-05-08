#include "dot_print.hpp"

void GraphDump::header_write() const {
    os << "digraph AST {\n";
    os << "  node [shape=box, style=filled, fillcolor=lightgray];\n";
}

void GraphDump::footer_write() const {
    os << "}\n";
}

void GraphDump::edge_write(int from, int to) const {
    os << "  node" << from << " -> node" << to << ";\n";
}

void GraphDump::visit(NumberNode& node) {
    cur_node_id = next_id();
    os << "  node" << cur_node_id << " [label=\"Num: " << node.value << "\", fillcolor=lightblue];\n";
}

void GraphDump::visit(VariableNode& node) {
    cur_node_id = next_id();
    os << "  node" << cur_node_id << " [label=\"Var: " << node.name << "\", fillcolor=lightgreen];\n";
}

void GraphDump::visit(BinaryOpNode& node) {
    int id = next_id();
    os << "  node" << id << " [label=\"Op: " << get_op(node.op) << "\", fillcolor=gold];\n";

    node.left->accept(*this);
    edge_write(id, cur_node_id);

    node.right->accept(*this);
    edge_write(id, cur_node_id);

    cur_node_id = id;
}

void GraphDump::visit(AssignmentNode& node) {
    int id = next_id();
    os << "  node" << id << " [label=\"Assign: " << node.var_name << "\", fillcolor=salmon];\n";

    node.expr->accept(*this);
    edge_write(id, cur_node_id);

    cur_node_id = id;
}

void GraphDump::visit(PrintNode& node) {
    int id = next_id();
    os << "  node" << id << " [label=\"Print\", fillcolor=plum];\n";

    node.expr->accept(*this);
    edge_write(id, cur_node_id);

    cur_node_id = id;
}

void GraphDump::visit(BlockNode& node) {
    int id = next_id();
    os << "  node" << id << " [label=\"Block\", fillcolor=gray];\n";

    for (auto& stmt : node.statements) {
        stmt->accept(*this);
        edge_write(id, cur_node_id);
    }

    cur_node_id = id;
}

void GraphDump::visit(IfStmtNode& node) {
    int id = next_id();
    os << "  node" << id << " [label=\"If\", fillcolor=orange];\n";

    node.condition->accept(*this);
    edge_write(id, cur_node_id);

    node.true_block->accept(*this);
    edge_write(id, cur_node_id);

    if (node.false_block) {
        node.false_block->accept(*this);
        edge_write(id, cur_node_id);
    }
    cur_node_id = id;
}


std::string GraphDump::get_op(const BinOp& op) const {
    switch(op) {
        case BinOp::ADD: return "+";
        case BinOp::SUB: return "-";
        case BinOp::MUL: return "*";
        case BinOp::DIV: return "/";
        case BinOp::MOD: return "%";
        case BinOp::AND: return "&&";
        case BinOp::OR: return "||";
        case BinOp::ADD_ASSIGN: return "+=";
        case BinOp::SUB_ASSIGN: return "-=";
        case BinOp::MUL_ASSIGN: return "*=";
        case BinOp::DIV_ASSIGN: return "/=";
        case BinOp::MOD_ASSIGN: return "%=";
        case BinOp::EQ_ASSIGN: return "==";
        case BinOp::ASSIGN: return "=";
        case BinOp::NEQ_ASSIGN: return "!=";
        case BinOp::LOW: return "<";
        case BinOp::HIGH: return ">";
        case BinOp::LOW_EQ: return "<=";
        case BinOp::HIGH_EQ: return ">=";
        default: return "undef op";
    }
}

void GraphDump::visit(UnaryOpNode& node) {
    int id = next_id();

    std::string label = {};
    switch(node.op) {
        case UnaryOp::NOT: label = "!"; break;
        case UnaryOp::INC: label = "++"; break;
        case UnaryOp::DEC: label = "--"; break;
        default: label = "undef unop"; break;
    }
    os << "  node" << id << " [label=\"UnOp: " << label << "\", fillcolor=violet];\n";
    node.operand->accept(*this);
    edge_write(id, cur_node_id);
    cur_node_id = id;
}

void GraphDump::visit(CallExprNode& node) {
    int id = next_id();
    os << "  node" << id << " [label=\"Call: " << node.function_name << "\", fillcolor=lightskyblue];\n";
    for (auto& arg : node.arguments) {
        arg->accept(*this);
        edge_write(id, cur_node_id);
    }
    cur_node_id = id;
}

void GraphDump::visit(ReturnStmtNode& node) {
    int id = next_id();
    os << "  node" << id << " [label=\"Return\", fillcolor=tomato];\n";
    if (node.expr) {
        node.expr->accept(*this);
        edge_write(id, cur_node_id);
    }
    cur_node_id = id;
}

void GraphDump::visit(FnDeclNode& node) {
    int id = next_id();
    os << "  node" << id << " [label=\"Function: " << node.name << "\", fillcolor=mediumaquamarine];\n";

    for (auto& p : node.params) {
        int pid = next_id();
        os << "  node" << pid << " [label=\"Param: " << p.name << "\", shape=ellipse];\n";
        edge_write(id, pid);
    }

    node.body->accept(*this);
    edge_write(id, cur_node_id);
    cur_node_id = id;
}

void GraphDump::visit(BreakNode& node) {
    cur_node_id = next_id();
    os << "  node" << cur_node_id << " [label=\"break\", fillcolor=firebrick1];\n";
}

void GraphDump::visit(ContinueNode& node) {
    cur_node_id = next_id();
    os << "  node" << cur_node_id << " [label=\"continue\", fillcolor=firebrick1];\n";
}

void GraphDump::visit(ForNode& node) {
    int id = next_id();
    os << "  node" << id << "  [label=\"For\", fillcolor=cornflowerblue];\n";

    node.init->accept(*this);
    edge_write(id, cur_node_id);

    node.cond->accept(*this);
    edge_write(id, cur_node_id);

    node.step->accept(*this);
    edge_write(id, cur_node_id);

    node.body->accept(*this);
    edge_write(id, cur_node_id);

    cur_node_id = id;
}

void GraphDump::visit(WhileStmtNode& node) {
    int id = next_id();
    os << "  node" << id << " [label=\"While\", fillcolor=lightcoral];\n";

    node.condition->accept(*this);
    edge_write(id, cur_node_id);

    node.body->accept(*this);
    edge_write(id, cur_node_id);

    cur_node_id = id;
}

void GraphDump::visit(CompoundAssignNode& node) {
    int id = next_id();

    os << "  node" << id << " [label=\"CompAssign: " << node.var_name << " " << get_op(node.op) << "\", fillcolor=lightcoral];\n";

    node.expr->accept(*this);
    edge_write(id, cur_node_id);

    cur_node_id = id;
}
