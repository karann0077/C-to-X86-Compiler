#pragma once
#include "ast/ASTVisitor.h"
#include "ast/TranslationUnit.h"
#include "ast/Expressions/LiteralExpr.h"
#include "ast/Expressions/VariableExpr.h"
#include "ast/Expressions/BinaryExpr.h"
#include "ast/Statements/CompoundStmt.h"
#include "ast/Statements/ReturnStmt.h"
#include "ast/Statements/ExprStmt.h"
#include "ast/Statements/DeclStmt.h"
#include "ast/Declarations/FunctionDecl.h"
#include "ast/Declarations/VarDecl.h"
#include "sema/Scope.h"
#include "diagnostics/DiagnosticEngine.h"

namespace cppx86 {

class SemanticAnalyzer : public ASTVisitor {
public:
    SemanticAnalyzer(DiagnosticEngine& diags);

    void analyze(TranslationUnit& tu);

    void visit(TranslationUnit& node) override;
    
    void visit(FunctionDecl& node) override;
    void visit(VarDecl& node) override;
    void visit(FieldDecl& node) override;
    void visit(RecordDecl& node) override;
    
    // Statements
    void visit(CompoundStmt& node) override;
    void visit(ReturnStmt& node) override;
    void visit(ExprStmt& node) override;
    void visit(DeclStmt& node) override;

    // Expressions
    void visitExpr(Expr* node);
    void visit(LiteralExpr& node) override;
    void visit(VariableExpr& node) override;
    void visit(BinaryExpr& node) override;
    void visit(MemberExpr& node) override;

private:
    DiagnosticEngine& diags;
    Scope* currentScope = nullptr;

    void enterScope();
    void leaveScope();
    
    TypePtr currentReturnType = nullptr; // For checking return statements
};

} // namespace cppx86
