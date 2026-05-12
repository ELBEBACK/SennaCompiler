#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

enum class Opcode {
    ALLOCA, LOAD, STORE,
    ADD, SUB, MUL, DIV, MOD,
    CMP_EQ, CMP_NEQ, CMP_LT, CMP_GT, CMP_LEQ, CMP_GEQ,
    AND, OR, NOT,
    CALL, PRINT,
    JMP, BR, RET,
};

class Value {
public:
    std::string name;
    explicit Value(std::string n = "") : name(std::move(n)) {}
    virtual ~Value() = default;
};

class ConstantInt : public Value {
public:
    int64_t val;
    explicit ConstantInt(int64_t v) : val(v) {}
};

class IrParam : public Value {
public:
    explicit IrParam(std::string n) : Value(std::move(n)) {}
};

class BasicBlock;

class Instruction : public Value {
public:
    Opcode              opcode;
    std::vector<Value*> operands;
    std::string         fn_name;
    BasicBlock*         bb1 = nullptr;
    BasicBlock*         bb2 = nullptr;

    Instruction(std::string nm, Opcode op, std::vector<Value*> ops = {})
        : Value(std::move(nm)), opcode(op), operands(std::move(ops)) {}
};

class BasicBlock {
public:
    uint32_t                  id;
    std::string               label;
    std::vector<Instruction*> insts;
    std::vector<BasicBlock*>  succs;
    std::vector<BasicBlock*>  preds;

    void add_edge(BasicBlock* other) {
        succs.push_back(other);
        other->preds.push_back(this);
    }

    bool is_terminated() const {
        if (insts.empty()) return false;
        auto op = insts.back()->opcode;
        return op == Opcode::JMP || op == Opcode::BR || op == Opcode::RET;
    }
};

class Function {
public:
    std::string           name;
    std::vector<IrParam*> params;
    BasicBlock*           entry = nullptr;

    std::vector<std::unique_ptr<BasicBlock>> blocks;
    std::vector<std::unique_ptr<Value>>      values;

    BasicBlock*  new_block(const std::string& label = "");
    ConstantInt* get_const(int64_t v);
    IrParam*     add_param(const std::string& name);
    Instruction* emit(BasicBlock* bb, Opcode op, std::vector<Value*> ops = {});
    Instruction* emit_named(BasicBlock* bb, std::string name, Opcode op, std::vector<Value*> ops = {});
    void         prune_dead_blocks();

private:
    uint32_t    bb_cnt_   = 0;
    uint32_t    temp_cnt_ = 0;
    std::string fresh_temp();
};

class Module {
public:
    std::vector<std::unique_ptr<Function>> functions;
};