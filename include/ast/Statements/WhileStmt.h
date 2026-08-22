#pragma once
#include "ast/Stmt.h"
#include "ast/Expr.h"
#include <memory>

namespace cppx86 {

class WhileStmt : public Stmt {
public:
    WhileStmt(ExprPtr condition, StmtPtr body)
        : condition(std::move(condition)), body(std::move(body)) {}

    void accept(ASTVisitor& visitor) override;

    Expr* getCondition() const { return condition.get(); }
    Stmt* getBody() const { return body.get(); }

private:
    ExprPtr condition;
    StmtPtr body;
};

} // namespace cppx86
