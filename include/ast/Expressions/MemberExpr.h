#pragma once
#include "ast/Expr.h"

namespace cppx86 {

class MemberExpr : public Expr {
public:
    MemberExpr(ExprPtr base, std::string memberName, bool isArrow = false)
        : base(std::move(base)), memberName(std::move(memberName)), isArrow(isArrow) {}

    void accept(ASTVisitor& visitor) override;

    Expr* getBase() const { return base.get(); }
    const std::string& getMemberName() const { return memberName; }
    bool getIsArrow() const { return isArrow; }

private:
    ExprPtr base;
    std::string memberName;
    bool isArrow; // true for `->`, false for `.`
};

} // namespace cppx86
