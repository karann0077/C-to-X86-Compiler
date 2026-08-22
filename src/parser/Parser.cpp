#include "parser/Parser.h"
#include "ast/Declarations/FunctionDecl.h"
#include "ast/Declarations/VarDecl.h"

namespace cppx86 {

Parser::Parser(TokenStream& stream, DiagnosticEngine& diags)
    : stream(stream), diags(diags) {
    advance();
}

void Parser::advance() {
    currentToken = stream.next();
}

bool Parser::match(TokenKind kind) {
    if (currentToken.kind == kind) {
        advance();
        return true;
    }
    return false;
}

void Parser::expect(TokenKind kind, const std::string& message) {
    if (currentToken.kind == kind) {
        advance();
    } else {
        diags.error(currentToken.location, message);
    }
}

TranslationUnitPtr Parser::parse() {
    auto tu = std::make_unique<TranslationUnit>();
    while (currentToken.kind != TokenKind::EndOfFile) {
        auto decl = parseDeclaration();
        if (decl) {
            tu->addDeclaration(std::move(decl));
        } else {
            advance(); // skip to avoid infinite loop on error
        }
    }
    return tu;
}

DeclPtr Parser::parseDeclaration() {
    // Skeleton implementation
    return nullptr;
}

TypeNodePtr Parser::parseType() {
    return nullptr;
}

std::unique_ptr<CompoundStmt> Parser::parseBlock() {
    return nullptr;
}

StmtPtr Parser::parseStatement() {
    return nullptr;
}

ExprPtr Parser::parseExpression() {
    return nullptr;
}

} // namespace cppx86
