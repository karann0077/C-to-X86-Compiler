// src/lexer.cpp  (REPLACE your existing lexer.cpp with this)
#include "lexer.h"
#include <cctype>
#include <unordered_map>

static std::unordered_map<std::string, TokenKind> keywords = {
    {"int", TokenKind::KwInt},
    {"return", TokenKind::KwReturn},
    {"if", TokenKind::KwIf},
    {"else", TokenKind::KwElse},
    {"while", TokenKind::KwWhile}
};

Lexer::Lexer(const std::string &src_) : src(src_) {
    // initialize cur by reading the first token
    cur = Token{TokenKind::End, "", 0, line};
    cur = next();
}

void Lexer::skipWhitespace(){
    while(i < src.size()){
        char c = src[i];
        if(c == '\n') { line++; i++; continue; }
        if(isspace((unsigned char)c)) { i++; continue; }
        if(c == '/' && i+1 < src.size() && src[i+1] == '/'){
            // line comment
            i+=2;
            while(i < src.size() && src[i] != '\n') i++;
            continue;
        }
        if(c == '/' && i+1 < src.size() && src[i+1] == '*'){
            i+=2;
            while(i+1 < src.size() && !(src[i]=='*' && src[i+1]=='/')) {
                if(src[i]=='\n') line++;
                i++;
            }
            if(i+1 < src.size()) i+=2;
            continue;
        }
        break;
    }
}

bool Lexer::startsWith(const std::string &s){
    return src.compare(i, s.size(), s) == 0;
}

Token Lexer::makeToken(TokenKind k, const std::string &t){
    Token tk; tk.kind = k; tk.text = t; tk.number = 0; tk.line = line; return tk;
}

Token Lexer::next(){
    skipWhitespace();
    if(i >= src.size()){
        Token t = makeToken(TokenKind::End, "");
        cur = t;
        return cur;
    }
    char c = src[i];

    // multi-char ops
    if(startsWith("==")) { i+=2; Token t = makeToken(TokenKind::Eq, "=="); cur = t; return cur; }
    if(startsWith("!=")) { i+=2; Token t = makeToken(TokenKind::Neq, "!="); cur = t; return cur; }
    if(startsWith("<=")) { i+=2; Token t = makeToken(TokenKind::Le, "<="); cur = t; return cur; }
    if(startsWith(">=")) { i+=2; Token t = makeToken(TokenKind::Ge, ">="); cur = t; return cur; }
    if(startsWith("&&")) { i+=2; Token t = makeToken(TokenKind::And, "&&"); cur = t; return cur; }
    if(startsWith("||")) { i+=2; Token t = makeToken(TokenKind::Or, "||"); cur = t; return cur; }

    // single-char tokens
    i++;
    switch(c){
        case '+': { Token t = makeToken(TokenKind::Plus, "+"); cur = t; return cur; }
        case '-': { Token t = makeToken(TokenKind::Minus, "-"); cur = t; return cur; }
        case '*': { Token t = makeToken(TokenKind::Star, "*"); cur = t; return cur; }
        case '/': { Token t = makeToken(TokenKind::Slash, "/"); cur = t; return cur; }
        case '%': { Token t = makeToken(TokenKind::Percent, "%"); cur = t; return cur; }
        case '(': { Token t = makeToken(TokenKind::LParen, "("); cur = t; return cur; }
        case ')': { Token t = makeToken(TokenKind::RParen, ")"); cur = t; return cur; }
        case '{': { Token t = makeToken(TokenKind::LBrace, "{"); cur = t; return cur; }
        case '}': { Token t = makeToken(TokenKind::RBrace, "}"); cur = t; return cur; }
        case ';': { Token t = makeToken(TokenKind::Semicolon, ";"); cur = t; return cur; }
        case ',': { Token t = makeToken(TokenKind::Comma, ","); cur = t; return cur; }
        case '=': { Token t = makeToken(TokenKind::Assign, "="); cur = t; return cur; }
        case '<': { Token t = makeToken(TokenKind::Lt, "<"); cur = t; return cur; }
        case '>': { Token t = makeToken(TokenKind::Gt, ">"); cur = t; return cur; }
        default: break;
    }

    // number
    if(isdigit((unsigned char)c)){
        size_t j = i-1;
        while(j < src.size() && isdigit((unsigned char)src[j])) j++;
        std::string s = src.substr(i-1, j - (i-1));
        i = j;
        Token t = makeToken(TokenKind::Number, s);
        t.number = std::stoi(s);
        cur = t;
        return cur;
    }

    // identifier or keyword
    if(isalpha((unsigned char)c) || c == '_'){
        size_t j = i-1;
        while(j < src.size() && (isalnum((unsigned char)src[j]) || src[j]=='_')) j++;
        std::string s = src.substr(i-1, j - (i-1));
        i = j;
        auto it = keywords.find(s);
        if(it != keywords.end()){
            Token t = makeToken(it->second, s);
            cur = t;
            return cur;
        }
        Token t = makeToken(TokenKind::Identifier, s);
        cur = t;
        return cur;
    }

    Token t = makeToken(TokenKind::Unknown, std::string(1,c));
    cur = t;
    return cur;
}

Token Lexer::peek(){
    return cur;
}
