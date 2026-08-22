#pragma once
#include "ast/ASTNode.h"
#include <memory>

namespace cppx86 {

class Stmt : public ASTNode {
public:
    virtual ~Stmt() = default;
};

using StmtPtr = std::unique_ptr<Stmt>;

} // namespace cppx86
