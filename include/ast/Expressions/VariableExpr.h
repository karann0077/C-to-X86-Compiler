#pragma once
#include "ast/Expr.h"
#include <string>

namespace cppx86 {

class VariableExpr : public Expr {
public:
    VariableExpr(std::string name) : name(std::move(name)) {}

    void accept(ASTVisitor& visitor) override {}

    const std::string& getName() const { return name; }

private:
    std::string name;
};

} // namespace cppx86
