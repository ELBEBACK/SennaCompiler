#include "doms_print.hpp"


void DomTreePrinter::print(const Function& fn, const DomTree& dom) {
    os_ << "digraph \"DomTree_" << fn.name << "\" {\n";
    os_ << "  node [shape=record, fontname=\"Courier\", fontsize=10];\n";

    for (const auto& bb : fn.blocks)
        os_ << "  \"" << bb->label << "\";\n";

    for (const auto& bb : fn.blocks) {
        auto it = dom.idom.find(bb.get());
        if (it == dom.idom.end()) continue;
        os_ << "  \"" << it->second->label
            << "\" -> \""  << bb->label << "\";\n";
    }

    os_ << "}\n";
}


void DomFrontPrinter::print(const Function& fn, const DomFronts& fronts) {
    os_ << "digraph \"DomFront_" << fn.name << "\" {\n";
    os_ << "  node [shape=record, fontname=\"Courier\", fontsize=10];\n";

    for (const auto& bb : fn.blocks)
        os_ << "  \"" << bb->label << "\";\n";

    for (const auto& [bb, frontier] : fronts.df) {
        for (BasicBlock* y : frontier) {
            os_ << "  \"" << bb->label
                << "\" -> \"" << y->label << "\";\n";
        }
    }

    os_ << "}\n";
}