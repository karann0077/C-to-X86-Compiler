#pragma once
#include "ir/BasicBlock.h"
#include "ir/Instruction.h"
#include <memory>

namespace cppx86 {
namespace ir {

class IRBuilder {
public:
    IRBuilder() : insertBlock(nullptr), nextTempId(0) {}

    void setInsertPoint(BasicBlock* bb) { insertBlock = bb; }
    BasicBlock* getInsertBlock() const { return insertBlock; }

    Instruction* createAdd(Value* lhs, Value* rhs) {
        auto inst = std::make_unique<Instruction>(InstructionKind::Add, lhs->getType(), getTempName());
        inst->addOperand(lhs);
        inst->addOperand(rhs);
        return insert(std::move(inst));
    }

    Instruction* createSub(Value* lhs, Value* rhs) {
        auto inst = std::make_unique<Instruction>(InstructionKind::Sub, lhs->getType(), getTempName());
        inst->addOperand(lhs);
        inst->addOperand(rhs);
        return insert(std::move(inst));
    }

    Instruction* createRet(Value* val = nullptr) {
        auto inst = std::make_unique<Instruction>(InstructionKind::Ret, nullptr);
        if (val) {
            inst->addOperand(val);
        }
        return insert(std::move(inst));
    }
    
    Instruction* createAlloca(TypePtr type, const std::string& name = "") {
        auto inst = std::make_unique<Instruction>(InstructionKind::Alloca, std::move(type), name.empty() ? getTempName() : name);
        return insert(std::move(inst));
    }
    
    Instruction* createStore(Value* val, Value* ptr) {
        auto inst = std::make_unique<Instruction>(InstructionKind::Store, nullptr);
        inst->addOperand(val);
        inst->addOperand(ptr);
        return insert(std::move(inst));
    }
    
    Instruction* createLoad(TypePtr type, Value* ptr, const std::string& name = "") {
        auto inst = std::make_unique<Instruction>(InstructionKind::Load, std::move(type), name.empty() ? getTempName() : name);
        inst->addOperand(ptr);
        return insert(std::move(inst));
    }

private:
    BasicBlock* insertBlock;
    int nextTempId;

    std::string getTempName() {
        return std::to_string(nextTempId++);
    }

    Instruction* insert(std::unique_ptr<Instruction> inst) {
        Instruction* ptr = inst.get();
        if (insertBlock) {
            insertBlock->addInstruction(std::move(inst));
        }
        return ptr;
    }
};

} // namespace ir
} // namespace cppx86
