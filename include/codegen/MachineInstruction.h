#pragma once
#include <string>
#include <vector>

namespace cppx86 {
namespace codegen {

enum class X86Reg {
    RAX, RBX, RCX, RDX, RDI, RSI, RBP, RSP,
    EAX, EBX, ECX, EDX, EDI, ESI,
    R8, R9, R10, R11,
    R8D, R9D, R10D, R11D
};

enum class OperandKind {
    Register,
    Immediate,
    Memory, // e.g. [rbp - 4]
    Label
};

struct MachineOperand {
    OperandKind kind;
    X86Reg reg;
    int imm;
    std::string memStr;

    static MachineOperand Reg(X86Reg r) { return {OperandKind::Register, r, 0, ""}; }
    static MachineOperand Imm(int i) { return {OperandKind::Immediate, X86Reg::RAX, i, ""}; }
    static MachineOperand Mem(std::string m) { return {OperandKind::Memory, X86Reg::RAX, 0, std::move(m)}; }
    static MachineOperand Lbl(std::string m) { return {OperandKind::Label, X86Reg::RAX, 0, std::move(m)}; }

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
                    case X86Reg::R8: return "r8";
                    case X86Reg::R9: return "r9";
                    case X86Reg::R10: return "r10";
                    case X86Reg::R11: return "r11";
                    case X86Reg::R8D: return "r8d";
                    case X86Reg::R9D: return "r9d";
                    case X86Reg::R10D: return "r10d";
                    case X86Reg::R11D: return "r11d";
                }
            case OperandKind::Immediate: return std::to_string(imm);
            case OperandKind::Memory: return "DWORD PTR [" + memStr + "]";
            case OperandKind::Label: return memStr;
        }
        return "";
    }
};

enum class X86InstKind {
    MOV, ADD, SUB, IMUL, IDIV, RET, PUSH, POP, LABEL,
    JMP, CMP, JE, JNE
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
            case X86InstKind::JMP: res = "jmp"; break;
            case X86InstKind::CMP: res = "cmp"; break;
            case X86InstKind::JE: res = "je"; break;
            case X86InstKind::JNE: res = "jne"; break;
            default: res = "unknown"; break;
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
