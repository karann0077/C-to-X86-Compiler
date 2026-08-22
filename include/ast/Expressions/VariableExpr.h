#pragma once
#include "ast/Expr.h"
#include <string>

namespace cppx86 {

class VariableExpr : public Expr {
public:
    VariableExpr(std::string name) : name(std::move(name)), isField(false) {}

    void accept(ASTVisitor& visitor) override {
        visitor.visit(*this);
    }

    const std::string& getName() const { return name; }
    
    bool getIsField() const { return isField; }
    void setIsField(bool b) { isField = b; }

private:
    std::string name;
    bool isField;
};

} // namespace cppx86
