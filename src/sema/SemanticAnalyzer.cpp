#include "sema/SemanticAnalyzer.h"
#include "types/BuiltinType.h"

namespace cppx86 {

SemanticAnalyzer::SemanticAnalyzer(DiagnosticEngine& diags) : diags(diags) {
}

void SemanticAnalyzer::analyze(TranslationUnit& tu) {
    tu.accept(*this);
}

void SemanticAnalyzer::enterScope() {
    Scope* newScope = new Scope(currentScope);
    currentScope = newScope;
}

void SemanticAnalyzer::leaveScope() {
    Scope* oldScope = currentScope;
    currentScope = currentScope->getParent();
    delete oldScope; // In a real compiler, might use an arena allocator
}

void SemanticAnalyzer::visit(TranslationUnit& node) {
    enterScope(); // Global scope
    for (const auto& decl : node.getDeclarations()) {
        if (auto funcDecl = dynamic_cast<FunctionDecl*>(decl.get())) {
            visit(*funcDecl);
        } else if (auto varDecl = dynamic_cast<VarDecl*>(decl.get())) {
            visit(*varDecl);
        }
    }
    leaveScope();
}

void SemanticAnalyzer::visit(FunctionDecl& node) {
    // For now, assume all functions return int
    TypePtr retType = std::make_shared<BuiltinType>(BuiltinTypeKind::Int);
    
    Symbol sym(node.getName(), SymbolKind::Function, retType, &node);
    if (!currentScope->declare(sym)) {
        diags.error(node.getLocation(), "Redefinition of '" + node.getName() + "'");
    }

    currentReturnType = retType;
    
    if (node.getBody()) {
        enterScope();
        
        // TODO: add parameters to scope here
        
        visit(*node.getBody());
        leaveScope();
    }
    
    currentReturnType = nullptr;
}

void SemanticAnalyzer::visit(VarDecl& node) {
    // For now, assume all variables are int
    TypePtr type = std::make_shared<BuiltinType>(BuiltinTypeKind::Int);
    
    if (node.getInitializer()) {
        visitExpr(node.getInitializer());
        // TODO: Type check initializer against variable type
    }
    
    Symbol sym(node.getName(), SymbolKind::Variable, type, &node);
    if (!currentScope->declare(sym)) {
        diags.error(node.getLocation(), "Redefinition of '" + node.getName() + "'");
    }
}

void SemanticAnalyzer::visit(CompoundStmt& node) {
    enterScope();
    for (const auto& stmt : node.getStatements()) {
        if (auto ret = dynamic_cast<ReturnStmt*>(stmt.get())) {
            visit(*ret);
        } else if (auto cmp = dynamic_cast<CompoundStmt*>(stmt.get())) {
            visit(*cmp);
        } else if (auto exp = dynamic_cast<ExprStmt*>(stmt.get())) {
            visit(*exp);
        } else if (auto ds = dynamic_cast<DeclStmt*>(stmt.get())) {
            visit(*ds);
        }
    }
    leaveScope();
}

void SemanticAnalyzer::visit(ReturnStmt& node) {
    if (node.getExpr()) {
        visitExpr(node.getExpr());
        // TODO: Type check against currentReturnType
    } else if (currentReturnType && currentReturnType->getKind() == TypeKind::Builtin) {
        auto builtin = std::static_pointer_cast<BuiltinType>(currentReturnType);
        if (builtin->getBuiltinKind() != BuiltinTypeKind::Void) {
            diags.error(node.getLocation(), "Non-void function should return a value");
        }
    }
}

void SemanticAnalyzer::visit(ExprStmt& node) {
    if (node.getExpr()) {
        visitExpr(node.getExpr());
    }
}

void SemanticAnalyzer::visit(DeclStmt& node) {
    if (auto varDecl = dynamic_cast<VarDecl*>(node.getDecl())) {
        visit(*varDecl);
    } else if (auto funcDecl = dynamic_cast<FunctionDecl*>(node.getDecl())) {
        visit(*funcDecl);
    }
}

void SemanticAnalyzer::visitExpr(Expr* node) {
    if (auto lit = dynamic_cast<LiteralExpr*>(node)) {
        visit(*lit);
    } else if (auto var = dynamic_cast<VariableExpr*>(node)) {
        visit(*var);
    } else if (auto bin = dynamic_cast<BinaryExpr*>(node)) {
        visit(*bin);
    }
}

void SemanticAnalyzer::visit(LiteralExpr& node) {
    // For now, literals are ints
    node.setType(std::make_shared<BuiltinType>(BuiltinTypeKind::Int));
}

void SemanticAnalyzer::visit(VariableExpr& node) {
    auto sym = currentScope->lookup(node.getName());
    if (!sym) {
        diags.error(node.getLocation(), "Use of undeclared identifier '" + node.getName() + "'");
        node.setType(std::make_shared<BuiltinType>(BuiltinTypeKind::Int)); // fallback
    } else {
        node.setType(sym->type);
    }
}

void SemanticAnalyzer::visit(BinaryExpr& node) {
    visitExpr(node.getLHS());
    visitExpr(node.getRHS());
    
    // Type checking: ensure LHS and RHS are compatible
    // For now, assume int operations return int, comparisons return bool
    TypePtr resType = std::make_shared<BuiltinType>(BuiltinTypeKind::Int);
    if (node.getOp() == TokenKind::Equal || node.getOp() == TokenKind::Less || node.getOp() == TokenKind::Greater) {
        resType = std::make_shared<BuiltinType>(BuiltinTypeKind::Bool);
    }
    
    node.setType(resType);
}

} // namespace cppx86
