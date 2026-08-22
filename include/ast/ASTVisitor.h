#pragma once

namespace cppx86 {

class TranslationUnit;
// We will add more nodes here as we define them
// class VariableExpr;
// class BinaryExpr;

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;
    
    virtual void visit(TranslationUnit& node) = 0;
};

} // namespace cppx86
