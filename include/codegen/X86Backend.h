#pragma once
#include "ir/Module.h"
#include "codegen/MachineInstruction.h"
#include <iostream>
#include <unordered_map>
#include <string>

namespace cppx86 {
namespace codegen {

class X86Backend {
public:
    void generate(ir::Module& module, std::ostream& out);

private:
    void lowerFunction(ir::Function& func);
    void lowerBasicBlock(ir::BasicBlock& bb);
    void lowerInstruction(ir::Instruction& inst);

    std::vector<MachineInstruction> m_instructions;
    
    // Register Allocation State
    std::unordered_map<ir::Value*, int> stackOffsets; // For spilled values
    int currentStackOffset = 0;
    
    std::vector<X86Reg> freeRegisters;
    std::unordered_map<ir::Value*, X86Reg> valueToRegister;
    
    // Liveness (map value to the instruction index where it is last used)
    std::unordered_map<ir::Value*, int> lastUses;
    int currentInstIndex = 0;

    void computeLiveness(ir::BasicBlock& bb);
    void freeDeadRegisters(ir::Instruction& inst);

    void emitPrologue(const std::string& name);
    void emitEpilogue();
    
    MachineOperand getOperandForValue(ir::Value* val);
    
    void emit(X86InstKind kind, MachineOperand op1) {
        MachineInstruction mi(kind);
        mi.addOperand(op1);
        m_instructions.push_back(std::move(mi));
    }
    
    void emit(X86InstKind kind, MachineOperand op1, MachineOperand op2) {
        MachineInstruction mi(kind);
        mi.addOperand(op1);
        mi.addOperand(op2);
        m_instructions.push_back(std::move(mi));
    }
    
    void emit(X86InstKind kind) {
        m_instructions.push_back(MachineInstruction(kind));
    }
};

} // namespace codegen
} // namespace cppx86
