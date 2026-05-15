#pragma once
#include "ir.hpp"


class ConstFold {
public:
    bool run(Function& fn);
};
