#pragma once
#include "ast/Stmt.h"
#include "ast/Decl.h"

namespace cppx86 {

class DeclStmt : public Stmt {
public:
    DeclStmt(DeclPtr decl) : decl(std::move(decl)) {}

    void accept(ASTVisitor& visitor) override {}

    Decl* getDecl() const { return decl.get(); }

private:
    DeclPtr decl;
};

} // namespace cppx86
