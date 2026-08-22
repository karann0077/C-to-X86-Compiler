#pragma once
#include "ast/Stmt.h"
#include "ast/Expr.h"

namespace cppx86 {

class ReturnStmt : public Stmt {
public:
    ReturnStmt(ExprPtr expr) : expr(std::move(expr)) {}

    void accept(ASTVisitor& visitor) override {
        visitor.visit(*this);
    }

    Expr* getExpr() const { return expr.get(); }

private:
    ExprPtr expr; // can be null for void returns
};

} // namespace cppx86
