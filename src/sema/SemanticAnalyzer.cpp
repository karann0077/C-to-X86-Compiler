#include "sema/SemanticAnalyzer.h"
#include "types/BuiltinType.h"
#include "ast/Declarations/RecordDecl.h"
#include "ast/Declarations/FieldDecl.h"
#include "ast/Expressions/MemberExpr.h"

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
        } else if (auto recordDecl = dynamic_cast<RecordDecl*>(decl.get())) {
            visit(*recordDecl);
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

void SemanticAnalyzer::visit(FieldDecl& node) {
    // In a real compiler, fields exist within the class scope
    TypePtr type = std::make_shared<BuiltinType>(BuiltinTypeKind::Int); // simplified
    Symbol sym(node.getName(), SymbolKind::Variable, type, &node); // Fields are treated like variables in scope for now
    if (!currentScope->declare(sym)) {
        diags.error(node.getLocation(), "Redefinition of field '" + node.getName() + "'");
    }
}

void SemanticAnalyzer::visit(RecordDecl& node) {
    // A Record type represents the class itself
    TypePtr type = std::make_shared<BuiltinType>(BuiltinTypeKind::Void); // Simplified, should be RecordType
    Symbol sym(node.getName(), SymbolKind::Type, type, &node);
    
    if (!currentScope->declare(sym)) {
        diags.error(node.getLocation(), "Redefinition of '" + node.getName() + "'");
    }
    
    enterScope();
    // In C++, the implicit 'this' pointer is available in methods
    // Let's add it to the class scope so methods can find it.
    Symbol thisSym("this", SymbolKind::Variable, type, nullptr);
    currentScope->declare(thisSym);

    for (const auto& field : node.getFields()) {
        visit(*field);
    }
    for (const auto& method : node.getMethods()) {
        visit(*method);
    }
    leaveScope();
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
    } else if (auto mem = dynamic_cast<MemberExpr*>(node)) {
        visit(*mem);
    }
}

void SemanticAnalyzer::visit(MemberExpr& node) {
    visitExpr(node.getBase());
    // For now, assume it always resolves successfully to Int
    node.setType(std::make_shared<BuiltinType>(BuiltinTypeKind::Int));
}

void SemanticAnalyzer::visit(LiteralExpr& node) {
    // For now, literals are ints
    node.setType(std::make_shared<BuiltinType>(BuiltinTypeKind::Int));
}

void SemanticAnalyzer::visit(VariableExpr& node) {
    if (auto sym = currentScope->lookup(node.getName())) {
        node.setType(sym->type);
        
        // If the symbol is a field (or we assume it's a field if it's declared in a Record scope)
        // A simple heuristic for now: if 'this' is in scope, and it's not a local param/var, it might be a field.
        // Actually, we can check if it was declared in a class scope.
        // Let's assume all fields are marked by checking if it's not "this" and it exists in a scope above a function scope?
        // Let's add a quick hack for Phase 14: if the symbol's declaration is a FieldDecl.
        if (dynamic_cast<FieldDecl*>(sym->declaration)) {
            node.setIsField(true);
        }
    } else {
        diags.error(node.getLocation(), "Use of undeclared identifier '" + node.getName() + "'");
        node.setType(std::make_shared<BuiltinType>(BuiltinTypeKind::Int)); // fallback
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
