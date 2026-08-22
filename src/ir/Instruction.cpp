#include "ir/Instruction.h"
#include <sstream>

namespace cppx86 {
namespace ir {

// Replaced toString with print
std::string Instruction::print() const {
    std::stringstream ss;
    if (getType()) {
        ss << Value::toString() << " = ";
    }
    
    switch (kind) {
        case InstructionKind::Alloca: ss << "alloca"; break;
        case InstructionKind::Load: ss << "load"; break;
        case InstructionKind::Store: ss << "store"; break;
        case InstructionKind::Add: ss << "add"; break;
        case InstructionKind::Sub: ss << "sub"; break;
        case InstructionKind::Mul: ss << "mul"; break;
        case InstructionKind::Div: ss << "div"; break;
        case InstructionKind::CmpEq: ss << "cmpeq"; break;
        case InstructionKind::CmpLt: ss << "cmplt"; break;
        case InstructionKind::CmpGt: ss << "cmpgt"; break;
        case InstructionKind::Ret: ss << "ret"; break;
        case InstructionKind::Br: ss << "br"; break;
        case InstructionKind::CondBr: ss << "condbr"; break;
        case InstructionKind::Call: ss << "call"; break;
    }

    for (size_t i = 0; i < operands.size(); ++i) {
        ss << " " << operands[i]->toString();
        if (i + 1 < operands.size()) ss << ",";
    }

    return ss.str();
}

} // namespace ir
} // namespace cppx86
