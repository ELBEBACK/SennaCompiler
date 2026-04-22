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
    os << "  node" << cur_node_id << " [label=\"IntLit: " << node.value << "\", fillcolor=lightblue];\n";
}

void GraphDump::visit(VariableNode& node) {
    cur_node_id = next_id();
    os << "  node" << cur_node_id << " [label=\"Var: " << node.name << "\", fillcolor=lightgreen];\n";
}

void GraphDump::visit(BinaryOpNode& node) {
    int id = next_id();
    os << "  node" << id << " [label=\"Op: " << node.op << "\", fillcolor=gold];\n";

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
