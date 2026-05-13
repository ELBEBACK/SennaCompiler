#pragma once
#include "ir.hpp"
#include "dom_tree.hpp"
#include "dom_front.hpp"
#include <ostream>


class DomTreePrinter {
public:
    explicit DomTreePrinter(std::ostream& os) : os_(os) {}
    void print(const Function& fn, const DomTree& dom);

private:
    std::ostream& os_;
};


class DomFrontPrinter {
public:
    explicit DomFrontPrinter(std::ostream& os) : os_(os) {}
    void print(const Function& fn, const DomFronts& fronts);

private:
    std::ostream& os_;
};