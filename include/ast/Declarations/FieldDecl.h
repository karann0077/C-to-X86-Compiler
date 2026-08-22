#pragma once
#include "ast/Decl.h"
#include "ast/TypeNode.h"

namespace cppx86 {

class FieldDecl : public Decl {
public:
    FieldDecl(std::string name, TypeNodePtr type) 
        : Decl(std::move(name)), type(std::move(type)) {}

    void accept(ASTVisitor& visitor) override;
    
    TypeNode* getTypeNode() const { return type.get(); }

private:
    TypeNodePtr type;
};

using FieldDeclPtr = std::unique_ptr<FieldDecl>;

} // namespace cppx86
