#pragma once
#include "ast/ASTNode.h"
#include <memory>
#include <string>

namespace cppx86 {

class TypeNode : public ASTNode {
public:
    virtual ~TypeNode() = default;
};

using TypeNodePtr = std::unique_ptr<TypeNode>;

} // namespace cppx86
