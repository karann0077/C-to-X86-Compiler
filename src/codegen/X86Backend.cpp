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
    // If it's a constant (we hackily detect by lack of '%' in name for literals)
    // Wait, in IRGenerator we put the literal value in the name.
    // Let's improve the constant detection.
    if (val->getName().find_first_not_of("0123456789-") == std::string::npos && !val->getName().empty()) {
        return MachineOperand::Imm(std::stoi(val->getName()));
    }
    
    // Otherwise it's on the stack. Look it up.
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
        default: break; // Handle other instructions later
    }
}

void X86Backend::lowerBasicBlock(ir::BasicBlock& bb) {
    m_instructions.push_back(MachineInstruction(X86InstKind::LABEL, bb.getName()));
    for (const auto& inst : bb.getInstructions()) {
        lowerInstruction(*inst);
    }
}

void X86Backend::lowerFunction(ir::Function& func) {
    stackOffsets.clear();
    currentStackOffset = 0;

    emitPrologue(func.getName());
    
    for (const auto& bb : func.getBlocks()) {
        lowerBasicBlock(*bb);
    }
}

} // namespace codegen
} // namespace cppx86
