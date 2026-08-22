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
#include "ir/Module.h"
#include "ir/IRBuilder.h"
#include "sema/Scope.h"
#include <unordered_map>

namespace cppx86 {

class IRGenerator : public ASTVisitor {
public:
    IRGenerator();

    std::unique_ptr<ir::Module> generate(TranslationUnit& tu);

    void visit(TranslationUnit& node) override;
    void visit(FunctionDecl& node);
    void visit(VarDecl& node);
    
    void visit(CompoundStmt& node);
    void visit(ReturnStmt& node);
    void visit(ExprStmt& node);
    void visit(DeclStmt& node);

    void visitExpr(Expr* node);
    void visit(LiteralExpr& node);
    void visit(VariableExpr& node);
    void visit(BinaryExpr& node);

private:
    std::unique_ptr<ir::Module> module;
    ir::IRBuilder builder;
    ir::Function* currentFunction = nullptr;
    
    // Maps variable names to their alloca instructions in the current function
    std::unordered_map<std::string, ir::Value*> namedValues;
    
    // Result of the last expression visited
    ir::Value* lastValue = nullptr;
};

} // namespace cppx86
