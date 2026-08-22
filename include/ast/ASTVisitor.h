#pragma once

namespace cppx86 {

class TranslationUnit;
class FunctionDecl;
class VarDecl;
class FieldDecl;
class RecordDecl;

// Statements
class CompoundStmt;
class ReturnStmt;
class ExprStmt;
class DeclStmt;
class IfStmt;
class WhileStmt;

// Expressions
class LiteralExpr;
class VariableExpr;
class BinaryExpr;
class MemberExpr;

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    virtual void visit(TranslationUnit& node) {}
    
    // Declarations
    virtual void visit(FunctionDecl& node) {}
    virtual void visit(VarDecl& node) {}
    virtual void visit(FieldDecl& node) {}
    virtual void visit(RecordDecl& node) {}

    // Statements
    virtual void visit(CompoundStmt& node) {}
    virtual void visit(ReturnStmt& node) {}
    virtual void visit(ExprStmt& node) {}
    virtual void visit(DeclStmt& node) {}
    virtual void visit(IfStmt& node) {}
    virtual void visit(WhileStmt& node) {}

    // Expressions
    virtual void visit(LiteralExpr& node) {}
    virtual void visit(VariableExpr& node) {}
    virtual void visit(BinaryExpr& node) {}
    virtual void visit(MemberExpr& node) {}
};

} // namespace cppx86
