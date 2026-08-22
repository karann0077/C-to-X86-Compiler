#pragma once
#include "ast/Expr.h"
#include "lexer/Token.h"

namespace cppx86 {

class BinaryExpr : public Expr {
public:
    BinaryExpr(TokenKind op, ExprPtr lhs, ExprPtr rhs)
        : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
    
    void accept(ASTVisitor& visitor) override {}

    TokenKind getOp() const { return op; }
    Expr* getLHS() const { return lhs.get(); }
    Expr* getRHS() const { return rhs.get(); }

private:
    TokenKind op;
    ExprPtr lhs;
    ExprPtr rhs;
};

} // namespace cppx86
