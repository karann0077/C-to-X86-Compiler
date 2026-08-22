#pragma once
#include "ir/Value.h"
#include "ir/Instruction.h"
#include <vector>
#include <memory>

namespace cppx86 {
namespace ir {

class Function;

class BasicBlock : public Value {
public:
    BasicBlock(std::string name, Function* parent)
        : Value(nullptr, std::move(name)), parent(parent) {}

    void addInstruction(std::unique_ptr<Instruction> inst) {
        inst->setParent(this);
        instructions.push_back(std::move(inst));
    }

    const std::vector<std::unique_ptr<Instruction>>& getInstructions() const {
        return instructions;
    }

    Function* getParent() const { return parent; }

    std::string toString() const override {
        return getName() + ":"; // Label
    }

private:
    std::vector<std::unique_ptr<Instruction>> instructions;
    Function* parent;
};

} // namespace ir
} // namespace cppx86
