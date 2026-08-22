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
    
    // Declarations
    void visit(FunctionDecl& node);
    void visit(VarDecl& node);
    
    // Statements
    void visit(CompoundStmt& node);
    void visit(ReturnStmt& node);
    void visit(ExprStmt& node);
    void visit(DeclStmt& node);

    // Expressions
    void visitExpr(Expr* node);
    void visit(LiteralExpr& node);
    void visit(VariableExpr& node);
    void visit(BinaryExpr& node);

private:
    DiagnosticEngine& diags;
    Scope* currentScope = nullptr;

    void enterScope();
    void leaveScope();
    
    TypePtr currentReturnType = nullptr; // For checking return statements
};

} // namespace cppx86
