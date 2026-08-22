#pragma once
#include "ast/Stmt.h"
#include "ast/Expr.h"

namespace cppx86 {

class ExprStmt : public Stmt {
public:
    ExprStmt(ExprPtr expr) : expr(std::move(expr)) {}

    void accept(ASTVisitor& visitor) override {
        visitor.visit(*this);
    }

    Expr* getExpr() const { return expr.get(); }

private:
    ExprPtr expr;
};

} // namespace cppx86
