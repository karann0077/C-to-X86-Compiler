#pragma once
#include "opt/Pass.h"
#include <vector>

namespace cppx86 {
namespace opt {

class DeadCodeEliminationPass : public Pass {
public:
    bool runOnModule(ir::Module& module) override {
        bool changed = false;

        for (const auto& func : module.getFunctions()) {
            for (const auto& bb : func->getBlocks()) {
                std::vector<ir::Instruction*> toRemove;
                
                for (const auto& instPtr : bb->getInstructions()) {
                    ir::Instruction* inst = instPtr.get();
                    
                    // Instructions with side effects cannot be removed
                    if (inst->getInstKind() == ir::InstructionKind::Store ||
                        inst->getInstKind() == ir::InstructionKind::Ret ||
                        inst->getInstKind() == ir::InstructionKind::Br ||
                        inst->getInstKind() == ir::InstructionKind::CondBr ||
                        inst->getInstKind() == ir::InstructionKind::Call) {
                        continue;
                    }
                    
                    // Count uses
                    int uses = countUses(module, inst);
                    if (uses == 0) {
                        toRemove.push_back(inst);
                    }
                }
                
                for (ir::Instruction* inst : toRemove) {
                    bb->removeInstruction(inst);
                    changed = true;
                }
            }
        }
        
        return changed;
    }

private:
    int countUses(ir::Module& module, ir::Value* target) {
        int count = 0;
        for (const auto& func : module.getFunctions()) {
            for (const auto& bb : func->getBlocks()) {
                for (const auto& inst : bb->getInstructions()) {
                    for (size_t i = 0; i < inst->getNumOperands(); ++i) {
                        if (inst->getOperand(i) == target) {
                            count++;
                        }
                    }
                }
            }
        }
        return count;
    }
};

} // namespace opt
} // namespace cppx86
