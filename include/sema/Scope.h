#pragma once
#include "sema/Symbol.h"
#include <unordered_map>
#include <memory>
#include <optional>

namespace cppx86 {

class Scope {
public:
    Scope(Scope* parent = nullptr) : parent(parent) {}

    bool declare(const Symbol& symbol) {
        if (symbols.find(symbol.name) != symbols.end()) {
            return false; // Already declared in this scope
        }
        symbols[symbol.name] = symbol;
        return true;
    }

    std::optional<Symbol> lookup(const std::string& name) const {
        auto it = symbols.find(name);
        if (it != symbols.end()) {
            return it->second;
        }
        if (parent) {
            return parent->lookup(name);
        }
        return std::nullopt;
    }

    std::optional<Symbol> lookupCurrentScope(const std::string& name) const {
        auto it = symbols.find(name);
        if (it != symbols.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    Scope* getParent() const { return parent; }

private:
    Scope* parent;
    std::unordered_map<std::string, Symbol> symbols;
};

using ScopePtr = std::unique_ptr<Scope>;

} // namespace cppx86
