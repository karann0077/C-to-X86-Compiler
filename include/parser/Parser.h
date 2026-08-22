#pragma once
#include "lexer/Token.h"
#include "diagnostics/DiagnosticEngine.h"
#include "ast/TranslationUnit.h"
#include "ast/TypeNode.h"
#include "ast/Statements/CompoundStmt.h"
#include "ast/Expr.h"
#include <memory>

namespace cppx86 {

class Parser {
public:
    Parser(TokenStream& stream, DiagnosticEngine& diags);

    TranslationUnitPtr parse();

private:
    TokenStream& stream;
    DiagnosticEngine& diags;

    Token currentToken;
    void advance();
    bool match(TokenKind kind);
    void expect(TokenKind kind, const std::string& message);

    DeclPtr parseDeclaration();
    TypeNodePtr parseType();
    std::unique_ptr<CompoundStmt> parseBlock();
    StmtPtr parseStatement();
    ExprPtr parseExpression();
};

} // namespace cppx86
