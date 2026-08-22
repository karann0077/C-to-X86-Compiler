#pragma once
#include "ast/ASTVisitor.h"
#include "ast/TranslationUnit.h"
#include "ast/Expressions/LiteralExpr.h"
#include "ast/Expressions/BinaryExpr.h"
#include "ast/Expressions/VariableExpr.h"
#include "ast/Statements/CompoundStmt.h"
#include "ast/Statements/ReturnStmt.h"
#include "ast/Statements/ExprStmt.h"
#include "ast/Statements/DeclStmt.h"
#include "ast/Declarations/FunctionDecl.h"
#include "ast/Declarations/VarDecl.h"
#include <iostream>
#include <string>

namespace cppx86 {

class ASTPrinter : public ASTVisitor {
public:
    void print(TranslationUnit& tu) {
        tu.accept(*this);
    }

    void visit(TranslationUnit& node) override {
        std::cout << "TranslationUnit\n";
        indent++;
        for (const auto& decl : node.getDeclarations()) {
            if (auto funcDecl = dynamic_cast<FunctionDecl*>(decl.get())) {
                visit(*funcDecl);
            } else if (auto varDecl = dynamic_cast<VarDecl*>(decl.get())) {
                visit(*varDecl);
            }
        }
        indent--;
    }

    void visit(FunctionDecl& node) {
        printIndent();
        std::cout << "FunctionDecl: " << node.getName() << "\n";
        indent++;
        if (node.getBody()) {
            visit(*node.getBody());
        }
        indent--;
    }

    void visit(VarDecl& node) {
        printIndent();
        std::cout << "VarDecl: " << node.getName() << "\n";
        indent++;
        if (node.getInitializer()) {
            visitExpr(node.getInitializer());
        }
        indent--;
    }

    void visit(CompoundStmt& node) {
        printIndent();
        std::cout << "CompoundStmt\n";
        indent++;
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
        indent--;
    }

    void visit(DeclStmt& node) {
        printIndent();
        std::cout << "DeclStmt\n";
        indent++;
        if (auto varDecl = dynamic_cast<VarDecl*>(node.getDecl())) {
            visit(*varDecl);
        } else if (auto funcDecl = dynamic_cast<FunctionDecl*>(node.getDecl())) {
            visit(*funcDecl);
        }
        indent--;
    }

    void visit(ReturnStmt& node) {
        printIndent();
        std::cout << "ReturnStmt\n";
        indent++;
        if (node.getExpr()) {
            visitExpr(node.getExpr());
        }
        indent--;
    }

    void visit(ExprStmt& node) {
        printIndent();
        std::cout << "ExprStmt\n";
        indent++;
        if (node.getExpr()) {
            visitExpr(node.getExpr());
        }
        indent--;
    }

    void visitExpr(Expr* expr) {
        if (!expr) return;
        if (auto lit = dynamic_cast<LiteralExpr*>(expr)) {
            printIndent();
            std::cout << "LiteralExpr: " << lit->getValue() << "\n";
        } else if (auto var = dynamic_cast<VariableExpr*>(expr)) {
            printIndent();
            std::cout << "VariableExpr: " << var->getName() << "\n";
        } else if (auto bin = dynamic_cast<BinaryExpr*>(expr)) {
            printIndent();
            std::cout << "BinaryExpr\n";
            indent++;
            visitExpr(bin->getLHS());
            visitExpr(bin->getRHS());
            indent--;
        }
    }

private:
    int indent = 0;

    void printIndent() {
        for (int i = 0; i < indent; ++i) {
            std::cout << "  ";
        }
    }
};

} // namespace cppx86
