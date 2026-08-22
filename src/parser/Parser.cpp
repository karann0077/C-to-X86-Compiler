#include "parser/Parser.h"
#include "ast/Declarations/FunctionDecl.h"
#include "ast/Declarations/VarDecl.h"
#include "ast/Declarations/RecordDecl.h"
#include "ast/Declarations/FieldDecl.h"
#include "ast/Expressions/LiteralExpr.h"
#include "ast/Expressions/VariableExpr.h"
#include "ast/Expressions/BinaryExpr.h"
#include "ast/Expressions/MemberExpr.h"
#include "ast/Statements/CompoundStmt.h"
#include "ast/Statements/ReturnStmt.h"
#include "ast/Statements/ExprStmt.h"
#include "ast/Statements/DeclStmt.h"
#include "ast/Statements/IfStmt.h"
#include "ast/Statements/WhileStmt.h"

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
        case TokenKind::Dot:
        case TokenKind::Arrow: return 120;
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

DeclPtr Parser::parseRecordDecl() {
    bool isClass = (currentToken.kind == TokenKind::KwClass);
    advance(); // consume class/struct

    if (currentToken.kind != TokenKind::Identifier) {
        diags.error(currentToken.location, "Expected identifier after class/struct");
        return nullptr;
    }
    
    std::string name = currentToken.text;
    advance();
    
    auto record = std::make_unique<RecordDecl>(name, isClass);
    
    if (!match(TokenKind::LBrace)) {
        // Forward declaration
        expect(TokenKind::Semicolon, "Expected ';' after forward declaration");
        return record;
    }
    
    while (currentToken.kind != TokenKind::RBrace && currentToken.kind != TokenKind::EndOfFile) {
        // For now, ignore access specifiers (public, private)
        if (currentToken.kind == TokenKind::KwPublic || currentToken.kind == TokenKind::KwPrivate || currentToken.kind == TokenKind::KwProtected) {
            advance();
            expect(TokenKind::Colon, "Expected ':' after access specifier");
            continue;
        }
        
        TypeNodePtr type = parseType();
        if (!type) {
            diags.error(currentToken.location, "Expected a type specifier in record");
            advance();
            continue;
        }

        if (currentToken.kind != TokenKind::Identifier) {
            diags.error(currentToken.location, "Expected member name");
            advance();
            continue;
        }
        
        std::string memberName = currentToken.text;
        advance();
        
        if (match(TokenKind::LParen)) {
            // Method
            auto method = std::make_unique<FunctionDecl>(memberName, std::move(type));
            method->setIsMethod(true);
            // skip params for brevity...
            while (currentToken.kind != TokenKind::RParen && currentToken.kind != TokenKind::EndOfFile) advance();
            expect(TokenKind::RParen, "Expected ')'");
            
            if (currentToken.kind == TokenKind::LBrace) {
                method->setBody(parseBlock());
            } else {
                expect(TokenKind::Semicolon, "Expected ';' after method declaration");
            }
            record->addMethod(std::move(method));
        } else {
            // Field
            auto field = std::make_unique<FieldDecl>(memberName, std::move(type));
            expect(TokenKind::Semicolon, "Expected ';' after field declaration");
            record->addField(std::move(field));
        }
    }
    
    expect(TokenKind::RBrace, "Expected '}' at end of class/struct");
    expect(TokenKind::Semicolon, "Expected ';' after class/struct definition");
    
    return record;
}

DeclPtr Parser::parseDeclaration() {
    if (currentToken.kind == TokenKind::KwClass || currentToken.kind == TokenKind::KwStruct) {
        return parseRecordDecl();
    }

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
    
    if (currentToken.kind == TokenKind::KwIf) {
        advance();
        expect(TokenKind::LParen, "Expected '(' after 'if'");
        ExprPtr cond = parseExpression();
        expect(TokenKind::RParen, "Expected ')' after if condition");
        
        StmtPtr thenBlock = parseStatement();
        StmtPtr elseBlock = nullptr;
        
        if (match(TokenKind::KwElse)) {
            elseBlock = parseStatement();
        }
        
        return std::make_unique<IfStmt>(std::move(cond), std::move(thenBlock), std::move(elseBlock));
    }
    
    if (currentToken.kind == TokenKind::KwWhile) {
        advance();
        expect(TokenKind::LParen, "Expected '(' after 'while'");
        ExprPtr cond = parseExpression();
        expect(TokenKind::RParen, "Expected ')' after while condition");
        
        StmtPtr body = parseStatement();
        
        return std::make_unique<WhileStmt>(std::move(cond), std::move(body));
    }

    if (currentToken.kind == TokenKind::KwInt || currentToken.kind == TokenKind::KwVoid || currentToken.kind == TokenKind::KwFloat) {
        auto decl = parseDeclaration();
        if (decl) {
            return std::make_unique<DeclStmt>(std::move(decl));
        }
        return nullptr;
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

        if (op == TokenKind::Dot || op == TokenKind::Arrow) {
            if (currentToken.kind != TokenKind::Identifier) {
                diags.error(currentToken.location, "Expected member name");
                return lhs;
            }
            std::string memberName = currentToken.text;
            advance();
            lhs = std::make_unique<MemberExpr>(std::move(lhs), memberName, op == TokenKind::Arrow);
        } else {
            ExprPtr rhs = parseExpressionHelper(tokPrec + 1);
            if (!rhs) return lhs;
            lhs = std::make_unique<BinaryExpr>(op, std::move(lhs), std::move(rhs));
        }
    }
}

ExprPtr Parser::parseExpression() {
    return parseExpressionHelper(0);
}

} // namespace cppx86
