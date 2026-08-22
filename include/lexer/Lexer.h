#pragma once
#include "lexer/Token.h"
#include "diagnostics/DiagnosticEngine.h"
#include <string>

namespace cppx86 {

class Lexer : public TokenStream {
public:
    Lexer(std::string source, std::string filename, DiagnosticEngine& diags);

    Token next() override;
    Token peek() override;

private:
    std::string source;
    std::string filename;
    DiagnosticEngine& diags;

    std::size_t offset = 0;
    int line = 1;
    int column = 1;

    Token currentToken;
    bool hasPeeked = false;

    Token advanceToken();
    char peekChar() const;
    char nextChar();
    bool isAtEnd() const;
    bool match(char expected);

    void skipWhitespaceAndComments();
    
    Token identifierOrKeyword();
    Token number();
    Token stringLiteral();
    Token charLiteral();

    SourceLocation currentLocation() const;
    Token makeToken(TokenKind kind, std::string text);
    Token errorToken(std::string message);
};

} // namespace cppx86
