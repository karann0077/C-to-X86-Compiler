#pragma once
#include "ast/ASTNode.h"
#include <string>
#include <memory>

namespace cppx86 {

class Decl : public ASTNode {
public:
    Decl(std::string name) : name(std::move(name)) {}
    virtual ~Decl() = default;

    const std::string& getName() const { return name; }

private:
    std::string name;
};

using DeclPtr = std::unique_ptr<Decl>;

} // namespace cppx86
