#pragma once
#include <string>
#include <vector>

enum class TokenKind {
    End, Identifier, Number,
    KwInt, KwReturn, KwIf, KwElse, KwWhile,
    Plus, Minus, Star, Slash, Percent,
    LParen, RParen, LBrace, RBrace, Semicolon, Comma,
    Assign, Eq, Neq, Lt, Le, Gt, Ge,
    And, Or,
    Unknown
};

struct Token {
    TokenKind kind;
    std::string text;
    int number; // if Number
    int line;
};

class Lexer {
public:
    Lexer(const std::string &src);
    Token next();
    Token peek();
private:
    const std::string src;
    size_t i = 0;
    int line = 1;
    Token cur;
    void skipWhitespace();
    bool startsWith(const std::string &s);
    Token makeToken(TokenKind k, const std::string &t);
};
