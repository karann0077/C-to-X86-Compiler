#pragma once
#include "ast/Decl.h"
#include "ast/TypeNode.h"
#include "ast/Expr.h"

namespace cppx86 {

class VarDecl : public Decl {
public:
    VarDecl(std::string name, TypeNodePtr type, ExprPtr init = nullptr) 
        : Decl(std::move(name)), type(std::move(type)), initializer(std::move(init)) {}

    void accept(ASTVisitor& visitor) override {}

private:
    TypeNodePtr type;
    ExprPtr initializer;
};

} // namespace cppx86
