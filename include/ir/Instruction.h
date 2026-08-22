#pragma once
#include "ir/Value.h"
#include <vector>

namespace cppx86 {
namespace ir {

class BasicBlock;

enum class InstructionKind {
    Alloca,
    Load,
    Store,
    Add,
    Sub,
    Mul,
    Div,
    CmpEq,
    CmpLt,
    CmpGt,
    Ret,
    Br,
    CondBr,
    Call
};

class Instruction : public Value {
public:
    Instruction(InstructionKind kind, TypePtr type, std::string name = "")
        : Value(std::move(type), std::move(name)), kind(kind) {}

    InstructionKind getInstKind() const { return kind; }
    
    void addOperand(Value* val) { operands.push_back(val); }
    Value* getOperand(size_t index) const { return operands.at(index); }
    size_t getNumOperands() const { return operands.size(); }

    void setParent(BasicBlock* bb) { parent = bb; }
    BasicBlock* getParent() const { return parent; }

    std::string print() const;

private:
    InstructionKind kind;
    std::vector<Value*> operands;
    BasicBlock* parent = nullptr;
};

} // namespace ir
} // namespace cppx86
