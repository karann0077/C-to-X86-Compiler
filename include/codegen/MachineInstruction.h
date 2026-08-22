#pragma once
#include <string>
#include <vector>

namespace cppx86 {
namespace codegen {

enum class X86Reg {
    RAX, RBX, RCX, RDX, RDI, RSI, RBP, RSP,
    EAX, EBX, ECX, EDX, EDI, ESI,
    // Add more as needed
};

enum class OperandKind {
    Register,
    Immediate,
    Memory // e.g. [rbp - 4]
};

struct MachineOperand {
    OperandKind kind;
    X86Reg reg;
    int imm;
    std::string memStr;

    static MachineOperand Reg(X86Reg r) { return {OperandKind::Register, r, 0, ""}; }
    static MachineOperand Imm(int i) { return {OperandKind::Immediate, X86Reg::RAX, i, ""}; }
    static MachineOperand Mem(std::string m) { return {OperandKind::Memory, X86Reg::RAX, 0, std::move(m)}; }

    std::string toString() const {
        switch (kind) {
            case OperandKind::Register:
                switch (reg) {
                    case X86Reg::RAX: return "rax";
                    case X86Reg::RBX: return "rbx";
                    case X86Reg::RCX: return "rcx";
                    case X86Reg::RDX: return "rdx";
                    case X86Reg::RDI: return "rdi";
                    case X86Reg::RSI: return "rsi";
                    case X86Reg::RBP: return "rbp";
                    case X86Reg::RSP: return "rsp";
                    case X86Reg::EAX: return "eax";
                    case X86Reg::EBX: return "ebx";
                    case X86Reg::ECX: return "ecx";
                    case X86Reg::EDX: return "edx";
                    case X86Reg::EDI: return "edi";
                    case X86Reg::ESI: return "esi";
                }
            case OperandKind::Immediate: return std::to_string(imm);
            case OperandKind::Memory: return "DWORD PTR [" + memStr + "]";
        }
        return "";
    }
};

enum class X86InstKind {
    MOV, ADD, SUB, IMUL, IDIV, RET, PUSH, POP, LABEL
};

class MachineInstruction {
public:
    MachineInstruction(X86InstKind kind, std::string label = "") : kind(kind), label(std::move(label)) {}
    
    void addOperand(MachineOperand op) { operands.push_back(std::move(op)); }
    
    std::string toString() const {
        if (kind == X86InstKind::LABEL) {
            return label + ":";
        }

        std::string res;
        switch (kind) {
            case X86InstKind::MOV: res = "mov"; break;
            case X86InstKind::ADD: res = "add"; break;
            case X86InstKind::SUB: res = "sub"; break;
            case X86InstKind::IMUL: res = "imul"; break;
            case X86InstKind::IDIV: res = "idiv"; break;
            case X86InstKind::RET: res = "ret"; break;
            case X86InstKind::PUSH: res = "push"; break;
            case X86InstKind::POP: res = "pop"; break;
            default: break;
        }

        if (!operands.empty()) {
            res += " ";
            for (size_t i = 0; i < operands.size(); ++i) {
                res += operands[i].toString();
                if (i + 1 < operands.size()) res += ", ";
            }
        }
        return res;
    }

private:
    X86InstKind kind;
    std::string label;
    std::vector<MachineOperand> operands;
};

} // namespace codegen
} // namespace cppx86
