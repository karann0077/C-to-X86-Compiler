#pragma once
#include "ast/ASTNode.h"
#include "types/Type.h"
#include <memory>

namespace cppx86 {

class Expr : public ASTNode {
public:
    virtual ~Expr() = default;
    
    TypePtr getType() const { return type; }
    void setType(TypePtr t) { type = std::move(t); }

private:
    TypePtr type; // Will be populated during Semantic Analysis
};

using ExprPtr = std::unique_ptr<Expr>;

} // namespace cppx86
