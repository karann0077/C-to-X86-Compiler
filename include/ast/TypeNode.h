#pragma once
#include "ast/ASTNode.h"
#include <memory>
#include <string>

namespace cppx86 {

class TypeNode : public ASTNode {
public:
    virtual ~TypeNode() = default;
    
    void accept(ASTVisitor& visitor) override {}
};

using TypeNodePtr = std::unique_ptr<TypeNode>;

} // namespace cppx86
