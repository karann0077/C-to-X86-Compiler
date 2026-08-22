#include "parser/Parser.h"
#include "ast/Declarations/FunctionDecl.h"
#include "ast/Declarations/VarDecl.h"
#include "ast/Expressions/LiteralExpr.h"
#include "ast/Expressions/VariableExpr.h"
#include "ast/Expressions/BinaryExpr.h"
#include "ast/Statements/CompoundStmt.h"
#include "ast/Statements/ReturnStmt.h"
#include "ast/Statements/ExprStmt.h"
#include "ast/Statements/DeclStmt.h"

namespace cppx86 {

static int getPrecedence(TokenKind kind) {
    switch (kind) {
        case TokenKind::Assign:
        case TokenKind::PlusAssign:
        case TokenKind::MinusAssign:
        case TokenKind::StarAssign:
        case TokenKind::SlashAssign:
            return 10;
        case TokenKind::LogicalOr: return 20;
        case TokenKind::LogicalAnd: return 30;
        case TokenKind::BitOr: return 40;
        case TokenKind::BitXor: return 50;
        case TokenKind::BitAnd: return 60;
        case TokenKind::Equal:
        case TokenKind::NotEqual: return 70;
        case TokenKind::Less:
        case TokenKind::LessEqual:
        case TokenKind::Greater:
        case TokenKind::GreaterEqual: return 80;
        case TokenKind::ShiftLeft:
        case TokenKind::ShiftRight: return 90;
        case TokenKind::Plus:
        case TokenKind::Minus: return 100;
        case TokenKind::Star:
        case TokenKind::Slash:
        case TokenKind::Percent: return 110;
        default: return -1;
    }
}

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
        // Error recovery: skip until semicolon or EOF
        while (currentToken.kind != TokenKind::Semicolon && currentToken.kind != TokenKind::EndOfFile) {
            advance();
        }
        if (currentToken.kind == TokenKind::Semicolon) {
            advance();
        }
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

TypeNodePtr Parser::parseType() {
    // Basic stub for parsing types (e.g. `int`)
    if (currentToken.kind == TokenKind::KwInt || currentToken.kind == TokenKind::KwVoid || currentToken.kind == TokenKind::KwFloat) {
        advance();
        return std::make_unique<TypeNode>(); // In reality, we'd store the type info
    }
    return nullptr;
}

DeclPtr Parser::parseDeclaration() {
    // e.g. int x; or int main() { ... }
    TypeNodePtr type = parseType();
    if (!type) {
        diags.error(currentToken.location, "Expected a type specifier");
        return nullptr;
    }

    if (currentToken.kind != TokenKind::Identifier) {
        diags.error(currentToken.location, "Expected identifier");
        return nullptr;
    }
    
    std::string name = currentToken.text;
    advance();

    if (match(TokenKind::LParen)) {
        // Function declaration
        auto func = std::make_unique<FunctionDecl>(name, std::move(type));
        
        // Parse parameters
        if (!match(TokenKind::RParen)) {
            do {
                TypeNodePtr paramType = parseType();
                if (!paramType) {
                    diags.error(currentToken.location, "Expected parameter type");
                    break;
                }
                if (currentToken.kind != TokenKind::Identifier) {
                    diags.error(currentToken.location, "Expected parameter name");
                    break;
                }
                func->addParameter(currentToken.text, std::move(paramType));
                advance();
            } while (match(TokenKind::Comma));
            expect(TokenKind::RParen, "Expected ')' after parameters");
        }

        if (currentToken.kind == TokenKind::LBrace) {
            func->setBody(parseBlock());
        } else {
            expect(TokenKind::Semicolon, "Expected ';' after function declaration");
        }
        return func;
    } else {
        // Variable declaration
        ExprPtr init = nullptr;
        if (match(TokenKind::Assign)) {
            init = parseExpression();
        }
        expect(TokenKind::Semicolon, "Expected ';' after variable declaration");
        return std::make_unique<VarDecl>(name, std::move(type), std::move(init));
    }
}

std::unique_ptr<CompoundStmt> Parser::parseBlock() {
    expect(TokenKind::LBrace, "Expected '{' to start block");
    auto block = std::make_unique<CompoundStmt>();
    
    while (currentToken.kind != TokenKind::RBrace && currentToken.kind != TokenKind::EndOfFile) {
        if (auto stmt = parseStatement()) {
            block->addStatement(std::move(stmt));
        }
    }
    
    expect(TokenKind::RBrace, "Expected '}' to end block");
    return block;
}

StmtPtr Parser::parseStatement() {
    if (currentToken.kind == TokenKind::KwInt || currentToken.kind == TokenKind::KwVoid || currentToken.kind == TokenKind::KwFloat) {
        auto decl = parseDeclaration();
        if (decl) {
            return std::make_unique<DeclStmt>(std::move(decl));
        }
        return nullptr;
    }

    if (currentToken.kind == TokenKind::KwReturn) {
        advance();
        ExprPtr expr = nullptr;
        if (currentToken.kind != TokenKind::Semicolon) {
            expr = parseExpression();
        }
        expect(TokenKind::Semicolon, "Expected ';' after return statement");
        return std::make_unique<ReturnStmt>(std::move(expr));
    }
    
    if (currentToken.kind == TokenKind::LBrace) {
        return parseBlock();
    }
    
    // Otherwise, expression statement
    ExprPtr expr = parseExpression();
    expect(TokenKind::Semicolon, "Expected ';' after expression statement");
    return std::make_unique<ExprStmt>(std::move(expr));
}

ExprPtr Parser::parsePrimary() {
    if (currentToken.kind == TokenKind::IntegerLiteral || 
        currentToken.kind == TokenKind::FloatLiteral ||
        currentToken.kind == TokenKind::StringLiteral) {
        auto expr = std::make_unique<LiteralExpr>(currentToken.text);
        advance();
        return expr;
    }
    
    if (currentToken.kind == TokenKind::Identifier) {
        auto expr = std::make_unique<VariableExpr>(currentToken.text);
        advance();
        return expr;
    }
    
    if (match(TokenKind::LParen)) {
        auto expr = parseExpression();
        expect(TokenKind::RParen, "Expected ')' after expression");
        return expr;
    }
    
    diags.error(currentToken.location, "Expected expression");
    return nullptr;
}

ExprPtr Parser::parseExpressionHelper(int exprPrec) {
    auto lhs = parsePrimary();
    if (!lhs) return nullptr;

    while (true) {
        int tokPrec = getPrecedence(currentToken.kind);
        if (tokPrec < exprPrec) {
            return lhs;
        }

        TokenKind op = currentToken.kind;
        advance();

        auto rhs = parseExpressionHelper(tokPrec + 1); // Left associative
        if (!rhs) return nullptr;

        lhs = std::make_unique<BinaryExpr>(op, std::move(lhs), std::move(rhs));
    }
}

ExprPtr Parser::parseExpression() {
    return parseExpressionHelper(0);
}

} // namespace cppx86
