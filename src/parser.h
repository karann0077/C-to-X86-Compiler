#pragma once
#include "lexer.h"
#include "ast.h"
#include <memory>

class Parser {
public:
    Parser(Lexer &lex);
    Program parse();
private:
    Lexer &lex;
    Token cur();
    Token consume();
    bool accept(TokenKind k);
    void expect(TokenKind k, const std::string &msg="");

    // parse helpers
    Function parseFunction();
    std::unique_ptr<BlockStmt> parseBlock();
    NodePtr parseStatement();
    NodePtr parseExpr();
    NodePtr parseAssignment();
    NodePtr parseLogicOr();
    NodePtr parseLogicAnd();
    NodePtr parseEquality();
    NodePtr parseRelational();
    NodePtr parseAddSub();
    NodePtr parseMulDiv();
    NodePtr parseUnary();
    NodePtr parsePrimary();
};
