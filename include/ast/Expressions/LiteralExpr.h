#pragma once
#include "ast/Expr.h"
#include <string>

namespace cppx86 {

class LiteralExpr : public Expr {
public:
    LiteralExpr(std::string value) : value(std::move(value)) {}
    
    void accept(ASTVisitor& visitor) override {
        // visitor.visit(*this);
    }
    
    const std::string& getValue() const { return value; }

private:
    std::string value;
};

} // namespace cppx86
