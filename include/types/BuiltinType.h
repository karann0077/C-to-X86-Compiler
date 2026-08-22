#pragma once
#include "types/Type.h"

namespace cppx86 {

enum class BuiltinTypeKind {
    Void,
    Bool,
    Char,
    Short,
    Int,
    Long,
    Float,
    Double
};

class BuiltinType : public Type {
public:
    BuiltinType(BuiltinTypeKind kind) : builtinKind(kind) {}

    TypeKind getKind() const override { return TypeKind::Builtin; }
    
    BuiltinTypeKind getBuiltinKind() const { return builtinKind; }

    bool isIntegerType() const override {
        return builtinKind == BuiltinTypeKind::Bool ||
               builtinKind == BuiltinTypeKind::Char ||
               builtinKind == BuiltinTypeKind::Short ||
               builtinKind == BuiltinTypeKind::Int ||
               builtinKind == BuiltinTypeKind::Long;
    }

    bool isFloatingType() const override {
        return builtinKind == BuiltinTypeKind::Float ||
               builtinKind == BuiltinTypeKind::Double;
    }

    std::string toString() const override {
        switch (builtinKind) {
            case BuiltinTypeKind::Void: return "void";
            case BuiltinTypeKind::Bool: return "bool";
            case BuiltinTypeKind::Char: return "char";
            case BuiltinTypeKind::Short: return "short";
            case BuiltinTypeKind::Int: return "int";
            case BuiltinTypeKind::Long: return "long";
            case BuiltinTypeKind::Float: return "float";
            case BuiltinTypeKind::Double: return "double";
        }
        return "unknown";
    }

private:
    BuiltinTypeKind builtinKind;
};

} // namespace cppx86
