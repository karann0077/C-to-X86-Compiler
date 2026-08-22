#pragma once
#include "ast/Stmt.h"
#include "ast/Expr.h"
#include <memory>

namespace cppx86 {

class IfStmt : public Stmt {
public:
    IfStmt(ExprPtr condition, StmtPtr thenBlock, StmtPtr elseBlock = nullptr)
        : condition(std::move(condition)), thenBlock(std::move(thenBlock)), elseBlock(std::move(elseBlock)) {}

    void accept(ASTVisitor& visitor) override;

    Expr* getCondition() const { return condition.get(); }
    Stmt* getThenBlock() const { return thenBlock.get(); }
    Stmt* getElseBlock() const { return elseBlock.get(); }

private:
    ExprPtr condition;
    StmtPtr thenBlock;
    StmtPtr elseBlock;
};

} // namespace cppx86
