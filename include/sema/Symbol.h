#pragma once
#include "types/Type.h"
#include <string>

namespace cppx86 {

class Decl; // Forward declaration

enum class SymbolKind {
    Variable,
    Function,
    Type
};

struct Symbol {
    std::string name;
    SymbolKind kind;
    TypePtr type;
    Decl* declaration; // Link back to the AST declaration

    Symbol() : kind(SymbolKind::Variable), declaration(nullptr) {}
    Symbol(std::string name, SymbolKind kind, TypePtr type, Decl* decl)
        : name(std::move(name)), kind(kind), type(std::move(type)), declaration(decl) {}
};

} // namespace cppx86
