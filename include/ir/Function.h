#pragma once
#include "ir/Value.h"
#include "ir/BasicBlock.h"
#include <vector>
#include <memory>

namespace cppx86 {
namespace ir {

class Module;

class Function : public Value {
public:
    Function(TypePtr type, std::string name, Module* parent)
        : Value(std::move(type), std::move(name)), parent(parent) {}

    BasicBlock* createBasicBlock(const std::string& name = "") {
        std::string bbName = name.empty() ? "bb" + std::to_string(blocks.size()) : name;
        auto bb = std::make_unique<BasicBlock>(bbName, this);
        BasicBlock* ptr = bb.get();
        blocks.push_back(std::move(bb));
        return ptr;
    }

    const std::vector<std::unique_ptr<BasicBlock>>& getBlocks() const {
        return blocks;
    }

    Module* getParent() const { return parent; }

    std::string toString() const override {
        return "@" + getName();
    }

private:
    std::vector<std::unique_ptr<BasicBlock>> blocks;
    Module* parent;
};

} // namespace ir
} // namespace cppx86
