#pragma once
#include "opt/Pass.h"
#include "ir/Instruction.h"
#include <string>

namespace cppx86 {
namespace opt {

class ConstantFoldingPass : public Pass {
public:
    bool runOnModule(ir::Module& module) override {
        bool changed = false;

        for (const auto& func : module.getFunctions()) {
            for (const auto& bb : func->getBlocks()) {
                for (const auto& instPtr : bb->getInstructions()) {
                    ir::Instruction* inst = instPtr.get();
                    if (inst->getInstKind() == ir::InstructionKind::Add || 
                        inst->getInstKind() == ir::InstructionKind::Sub) {
                        
                        ir::Value* lhs = inst->getOperand(0);
                        ir::Value* rhs = inst->getOperand(1);
                        
                        if (isConstant(lhs) && isConstant(rhs)) {
                            int l = getConstantValue(lhs);
                            int r = getConstantValue(rhs);
                            int res = 0;
                            
                            if (inst->getInstKind() == ir::InstructionKind::Add) res = l + r;
                            else if (inst->getInstKind() == ir::InstructionKind::Sub) res = l - r;
                            
                            ir::Value* newConst = new ir::Value(inst->getType(), std::to_string(res));
                            
                            // Replace all uses of 'inst' with 'newConst'
                            replaceAllUses(module, inst, newConst);
                            changed = true;
                        }
                    }
                }
            }
        }
        return changed;
    }

private:
    bool isConstant(ir::Value* val) {
        if (!val) return false;
        std::string name = val->getName();
        if (name.empty()) return false;
        return name.find_first_not_of("0123456789-") == std::string::npos;
    }

    int getConstantValue(ir::Value* val) {
        return std::stoi(val->getName());
    }

    void replaceAllUses(ir::Module& module, ir::Value* oldVal, ir::Value* newVal) {
        for (const auto& func : module.getFunctions()) {
            for (const auto& bb : func->getBlocks()) {
                for (const auto& inst : bb->getInstructions()) {
                    for (size_t i = 0; i < inst->getNumOperands(); ++i) {
                        if (inst->getOperand(i) == oldVal) {
                            inst->setOperand(i, newVal);
                        }
                    }
                }
            }
        }
    }
};

} // namespace opt
} // namespace cppx86
