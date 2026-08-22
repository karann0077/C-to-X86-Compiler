#include "codegen/X86Backend.h"

namespace cppx86 {
namespace codegen {

void X86Backend::generate(ir::Module& module, std::ostream& out) {
    out << ".intel_syntax noprefix\n";
    out << ".global main\n\n";

    for (const auto& func : module.getFunctions()) {
        lowerFunction(*func);
    }
    
    // Dump all instructions
    for (const auto& inst : m_instructions) {
        if (inst.toString() != "") {
            // Very hacky formatting for labels vs instructions
            std::string s = inst.toString();
            if (s.back() == ':') {
                out << s << "\n";
            } else {
                out << "    " << s << "\n";
            }
        }
    }
}

MachineOperand X86Backend::getOperandForValue(ir::Value* val) {
    if (val->getName().find_first_not_of("0123456789-") == std::string::npos && !val->getName().empty()) {
        return MachineOperand::Imm(std::stoi(val->getName()));
    }
    
    // Check if already in a register
    auto it = valueToRegister.find(val);
    if (it != valueToRegister.end()) {
        return MachineOperand::Reg(it->second);
    }
    
    // Allocate a register if available
    if (!freeRegisters.empty()) {
        X86Reg reg = freeRegisters.back();
        freeRegisters.pop_back();
        valueToRegister[val] = reg;
        return MachineOperand::Reg(reg);
    }
    
    // Spill to stack
    if (stackOffsets.find(val) == stackOffsets.end()) {
        currentStackOffset -= 4; // Allocate 4 bytes
        stackOffsets[val] = currentStackOffset;
    }
    
    int offset = stackOffsets[val];
    return MachineOperand::Mem("rbp" + (offset < 0 ? std::to_string(offset) : "+" + std::to_string(offset)));
}

void X86Backend::emitPrologue(const std::string& name) {
    m_instructions.push_back(MachineInstruction(X86InstKind::LABEL, name));
    emit(X86InstKind::PUSH, MachineOperand::Reg(X86Reg::RBP));
    emit(X86InstKind::MOV, MachineOperand::Reg(X86Reg::RBP), MachineOperand::Reg(X86Reg::RSP));
    // We will blindly allocate some stack space for now (e.g., 64 bytes)
    emit(X86InstKind::SUB, MachineOperand::Reg(X86Reg::RSP), MachineOperand::Imm(64));
}

void X86Backend::emitEpilogue() {
    emit(X86InstKind::MOV, MachineOperand::Reg(X86Reg::RSP), MachineOperand::Reg(X86Reg::RBP));
    emit(X86InstKind::POP, MachineOperand::Reg(X86Reg::RBP));
    emit(X86InstKind::RET);
}

void X86Backend::lowerInstruction(ir::Instruction& inst) {
    switch (inst.getInstKind()) {
        case ir::InstructionKind::Alloca: {
            // Memory is already reserved on the stack via getOperandForValue when used
            break;
        }
        case ir::InstructionKind::Store: {
            MachineOperand val = getOperandForValue(inst.getOperand(0));
            MachineOperand ptr = getOperandForValue(inst.getOperand(1));
            
            // X86 can't do Mem -> Mem directly, move to EAX first if val is Mem
            if (val.kind == OperandKind::Memory) {
                emit(X86InstKind::MOV, MachineOperand::Reg(X86Reg::EAX), val);
                val = MachineOperand::Reg(X86Reg::EAX);
            }
            emit(X86InstKind::MOV, ptr, val);
            break;
        }
        case ir::InstructionKind::Load: {
            MachineOperand ptr = getOperandForValue(inst.getOperand(0));
            MachineOperand dest = getOperandForValue(&inst);
            
            emit(X86InstKind::MOV, MachineOperand::Reg(X86Reg::EAX), ptr);
            emit(X86InstKind::MOV, dest, MachineOperand::Reg(X86Reg::EAX));
            break;
        }
        case ir::InstructionKind::Add: {
            MachineOperand lhs = getOperandForValue(inst.getOperand(0));
            MachineOperand rhs = getOperandForValue(inst.getOperand(1));
            MachineOperand dest = getOperandForValue(&inst);
            
            emit(X86InstKind::MOV, MachineOperand::Reg(X86Reg::EAX), lhs);
            if (rhs.kind == OperandKind::Memory) {
                emit(X86InstKind::MOV, MachineOperand::Reg(X86Reg::EBX), rhs);
                emit(X86InstKind::ADD, MachineOperand::Reg(X86Reg::EAX), MachineOperand::Reg(X86Reg::EBX));
            } else {
                emit(X86InstKind::ADD, MachineOperand::Reg(X86Reg::EAX), rhs);
            }
            emit(X86InstKind::MOV, dest, MachineOperand::Reg(X86Reg::EAX));
            break;
        }
        case ir::InstructionKind::Sub: {
            MachineOperand lhs = getOperandForValue(inst.getOperand(0));
            MachineOperand rhs = getOperandForValue(inst.getOperand(1));
            MachineOperand dest = getOperandForValue(&inst);
            
            emit(X86InstKind::MOV, MachineOperand::Reg(X86Reg::EAX), lhs);
            if (rhs.kind == OperandKind::Memory) {
                emit(X86InstKind::MOV, MachineOperand::Reg(X86Reg::EBX), rhs);
                emit(X86InstKind::SUB, MachineOperand::Reg(X86Reg::EAX), MachineOperand::Reg(X86Reg::EBX));
            } else {
                emit(X86InstKind::SUB, MachineOperand::Reg(X86Reg::EAX), rhs);
            }
            emit(X86InstKind::MOV, dest, MachineOperand::Reg(X86Reg::EAX));
            break;
        }
        case ir::InstructionKind::Ret: {
            if (inst.getNumOperands() > 0) {
                MachineOperand val = getOperandForValue(inst.getOperand(0));
                emit(X86InstKind::MOV, MachineOperand::Reg(X86Reg::EAX), val);
            }
            emitEpilogue();
            break;
        }
        case ir::InstructionKind::Br: {
            ir::BasicBlock* target = static_cast<ir::BasicBlock*>(inst.getOperand(0));
            emit(X86InstKind::JMP, MachineOperand::Lbl(target->getName()));
            break;
        }
        case ir::InstructionKind::CondBr: {
            MachineOperand cond = getOperandForValue(inst.getOperand(0));
            ir::BasicBlock* trueTarget = static_cast<ir::BasicBlock*>(inst.getOperand(1));
            ir::BasicBlock* falseTarget = static_cast<ir::BasicBlock*>(inst.getOperand(2));
            
            // Compare condition with 0
            emit(X86InstKind::CMP, cond, MachineOperand::Imm(0));
            // Jump to false target if equal to 0 (false)
            emit(X86InstKind::JE, MachineOperand::Lbl(falseTarget->getName()));
            // Otherwise jump to true target
            emit(X86InstKind::JMP, MachineOperand::Lbl(trueTarget->getName()));
            break;
        }
        default: break; // Handle other instructions later
    }
}

void X86Backend::computeLiveness(ir::BasicBlock& bb) {
    int index = 0;
    for (const auto& inst : bb.getInstructions()) {
        for (size_t i = 0; i < inst->getNumOperands(); ++i) {
            ir::Value* op = inst->getOperand(i);
            // We only care about tracking variables/instructions, not constants
            if (op->getName().find_first_not_of("0123456789-") != std::string::npos) {
                lastUses[op] = index;
            }
        }
        index++;
    }
}

void X86Backend::freeDeadRegisters(ir::Instruction& inst) {
    for (size_t i = 0; i < inst.getNumOperands(); ++i) {
        ir::Value* op = inst.getOperand(i);
        if (lastUses.find(op) != lastUses.end() && lastUses[op] == currentInstIndex) {
            // Free the register!
            auto it = valueToRegister.find(op);
            if (it != valueToRegister.end()) {
                freeRegisters.push_back(it->second);
                valueToRegister.erase(it);
            }
        }
    }
}

void X86Backend::lowerBasicBlock(ir::BasicBlock& bb) {
    m_instructions.push_back(MachineInstruction(X86InstKind::LABEL, bb.getName()));
    
    computeLiveness(bb);
    currentInstIndex = 0;
    
    for (const auto& inst : bb.getInstructions()) {
        lowerInstruction(*inst);
        freeDeadRegisters(*inst);
        currentInstIndex++;
    }
}

void X86Backend::lowerFunction(ir::Function& func) {
    stackOffsets.clear();
    valueToRegister.clear();
    lastUses.clear();
    currentStackOffset = 0;
    
    // Initialize register pool (using 32-bit registers for int operations)
    freeRegisters = { X86Reg::ECX, X86Reg::EDX, X86Reg::R8D, X86Reg::R9D, X86Reg::R10D, X86Reg::R11D };

    emitPrologue(func.getName());
    
    for (const auto& bb : func.getBlocks()) {
        lowerBasicBlock(*bb);
    }
}

} // namespace codegen
} // namespace cppx86
