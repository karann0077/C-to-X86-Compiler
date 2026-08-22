// src/parser.cpp
#include "parser.h"
#include <stdexcept>
#include <iostream>

Parser::Parser(Lexer &lex_): lex(lex_) {}

// helper: current token (does NOT advance)
Token Parser::cur(){ return lex.peek(); }

// consume and return the token we just advanced past
Token Parser::consume(){ Token t = lex.next(); (void)lex.peek(); return t; }

bool Parser::accept(TokenKind k){
    if(cur().kind == k){
        consume();
        return true;
    }
    return false;
}

void Parser::expect(TokenKind k, const std::string &msg){
    if(cur().kind != k){
        throw std::runtime_error("Parse error at line " + std::to_string(cur().line) + ": expected " + msg + ", got '" + cur().text + "'");
    }
    consume();
}

Program Parser::parse(){
    Program p;
    while(cur().kind != TokenKind::End){
        p.funcs.push_back(parseFunction());
    }
    return p;
}

Function Parser::parseFunction(){
    // only: int IDENT() { ... }
    expect(TokenKind::KwInt, "int");
    if(cur().kind != TokenKind::Identifier) throw std::runtime_error("expected function name");
    std::string name = cur().text;
    consume();
    expect(TokenKind::LParen, "(");
    expect(TokenKind::RParen, ")");
    auto block = parseBlock();
    Function f;
    f.name = name;
    // move statements from block into function body
    for(auto &s : block->stmts) f.body.push_back(std::move(s));
    return f;
}

std::unique_ptr<BlockStmt> Parser::parseBlock(){
    expect(TokenKind::LBrace, "{");
    auto blk = std::make_unique<BlockStmt>();
    while(cur().kind != TokenKind::RBrace && cur().kind != TokenKind::End){
        blk->stmts.push_back(parseStatement());
    }
    expect(TokenKind::RBrace, "}");
    return blk;
}

NodePtr Parser::parseStatement(){
    if(cur().kind == TokenKind::KwInt){
        consume();
        if(cur().kind != TokenKind::Identifier) throw std::runtime_error("expected identifier in decl");
        std::string name = cur().text; consume();
        NodePtr init = nullptr;
        if(accept(TokenKind::Assign)){
            init = parseExpr();
        }
        expect(TokenKind::Semicolon, ";");
        return std::make_unique<DeclStmt>(name, std::move(init));
    }
    if(cur().kind == TokenKind::KwReturn){
        consume();
        auto e = parseExpr();
        expect(TokenKind::Semicolon, ";");
        return std::make_unique<ReturnStmt>(std::move(e));
    }
    if(cur().kind == TokenKind::KwIf){
        consume();
        expect(TokenKind::LParen, "(");
        auto cond = parseExpr();
        expect(TokenKind::RParen, ")");
        auto thenS = parseStatement();
        NodePtr elseS = nullptr;
        if(accept(TokenKind::KwElse)){
            elseS = parseStatement();
        }
        return std::make_unique<IfStmt>(std::move(cond), std::move(thenS), std::move(elseS));
    }
    if(cur().kind == TokenKind::KwWhile){
        consume();
        expect(TokenKind::LParen, "(");
        auto cond = parseExpr();
        expect(TokenKind::RParen, ")");
        auto body = parseStatement();
        return std::make_unique<WhileStmt>(std::move(cond), std::move(body));
    }
    if(cur().kind == TokenKind::LBrace){
        return parseBlock();
    }
    // expression or assignment statement
    auto e = parseAssignment();
    expect(TokenKind::Semicolon, ";");
    return std::make_unique<ExprStmt>(std::move(e));
}

// ----- FIXED parseAssignment -----
// Now correctly checks if the left-hand side is a variable by dynamic_cast on left.get()
NodePtr Parser::parseAssignment(){
    auto left = parseLogicOr();
    if(accept(TokenKind::Assign)){
        // left must be VarExpr
        VarExpr *v = dynamic_cast<VarExpr*>(left.get());
        if(!v) throw std::runtime_error("Left side of assignment must be a variable (line " + std::to_string(cur().line) + ")");
        auto rhs = parseAssignment(); // right-associative
        return std::make_unique<Binary>("=", std::move(left), std::move(rhs));
    }
    return left;
}

NodePtr Parser::parseExpr(){ return parseAssignment(); }

NodePtr Parser::parseLogicOr(){
    auto node = parseLogicAnd();
    while(accept(TokenKind::Or)){
        node = std::make_unique<Binary>("||", std::move(node), parseLogicAnd());
    }
    return node;
}

NodePtr Parser::parseLogicAnd(){
    auto node = parseEquality();
    while(accept(TokenKind::And)){
        node = std::make_unique<Binary>("&&", std::move(node), parseEquality());
    }
    return node;
}

NodePtr Parser::parseEquality(){
    auto node = parseRelational();
    while(true){
        if(accept(TokenKind::Eq)){
            node = std::make_unique<Binary>("==", std::move(node), parseRelational());
        } else if(accept(TokenKind::Neq)){
            node = std::make_unique<Binary>("!=", std::move(node), parseRelational());
        } else break;
    }
    return node;
}

NodePtr Parser::parseRelational(){
    auto node = parseAddSub();
    while(true){
        if(accept(TokenKind::Lt)) node = std::make_unique<Binary>("<", std::move(node), parseAddSub());
        else if(accept(TokenKind::Le)) node = std::make_unique<Binary>("<=", std::move(node), parseAddSub());
        else if(accept(TokenKind::Gt)) node = std::make_unique<Binary>(">", std::move(node), parseAddSub());
        else if(accept(TokenKind::Ge)) node = std::make_unique<Binary>(">=", std::move(node), parseAddSub());
        else break;
    }
    return node;
}

NodePtr Parser::parseAddSub(){
    auto node = parseMulDiv();
    while(true){
        if(accept(TokenKind::Plus)) node = std::make_unique<Binary>("+", std::move(node), parseMulDiv());
        else if(accept(TokenKind::Minus)) node = std::make_unique<Binary>("-", std::move(node), parseMulDiv());
        else break;
    }
    return node;
}

NodePtr Parser::parseMulDiv(){
    auto node = parseUnary();
    while(true){
        if(accept(TokenKind::Star)) node = std::make_unique<Binary>("*", std::move(node), parseUnary());
        else if(accept(TokenKind::Slash)) node = std::make_unique<Binary>("/", std::move(node), parseUnary());
        else if(accept(TokenKind::Percent)) node = std::make_unique<Binary>("%", std::move(node), parseUnary());
        else break;
    }
    return node;
}

NodePtr Parser::parseUnary(){
    if(accept(TokenKind::Minus)){
        auto r = parseUnary();
        return std::make_unique<Binary>("neg", std::make_unique<Integer>(0), std::move(r));
    }
    return parsePrimary();
}

NodePtr Parser::parsePrimary(){
    Token t = cur();
    if(t.kind == TokenKind::Number){
        consume();
        return std::make_unique<Integer>(t.number);
    }
    if(t.kind == TokenKind::Identifier){
        consume();
        return std::make_unique<VarExpr>(t.text);
    }
    if(accept(TokenKind::LParen)){
        auto e = parseExpr();
        expect(TokenKind::RParen, ")");
        return e;
    }
    throw std::runtime_error("Unexpected token in primary: " + t.text + " at line " + std::to_string(t.line));
}
