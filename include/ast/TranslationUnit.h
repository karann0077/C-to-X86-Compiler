#pragma once
#include "ast/Decl.h"
#include <vector>
#include <memory>

namespace cppx86 {

class TranslationUnit : public ASTNode {
public:
    void addDeclaration(DeclPtr decl) {
        declarations.push_back(std::move(decl));
    }

    const std::vector<DeclPtr>& getDeclarations() const {
        return declarations;
    }

    void accept(ASTVisitor& visitor) override {
        visitor.visit(*this);
    }

private:
    std::vector<DeclPtr> declarations;
};

using TranslationUnitPtr = std::unique_ptr<TranslationUnit>;

} // namespace cppx86
