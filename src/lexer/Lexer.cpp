#include "lexer/Lexer.h"
#include <cctype>
#include <unordered_map>

namespace cppx86 {

static const std::unordered_map<std::string, TokenKind> keywords = {
    {"int", TokenKind::KwInt}, {"long", TokenKind::KwLong}, {"short", TokenKind::KwShort},
    {"char", TokenKind::KwChar}, {"float", TokenKind::KwFloat}, {"double", TokenKind::KwDouble},
    {"void", TokenKind::KwVoid}, {"bool", TokenKind::KwBool}, {"auto", TokenKind::KwAuto},
    {"const", TokenKind::KwConst}, {"static", TokenKind::KwStatic}, {"extern", TokenKind::KwExtern},
    {"class", TokenKind::KwClass}, {"struct", TokenKind::KwStruct}, {"public", TokenKind::KwPublic},
    {"private", TokenKind::KwPrivate}, {"protected", TokenKind::KwProtected},
    {"virtual", TokenKind::KwVirtual}, {"override", TokenKind::KwOverride},
    {"template", TokenKind::KwTemplate}, {"typename", TokenKind::KwTypename},
    {"using", TokenKind::KwUsing}, {"namespace", TokenKind::KwNamespace},
    {"if", TokenKind::KwIf}, {"else", TokenKind::KwElse}, {"while", TokenKind::KwWhile},
    {"for", TokenKind::KwFor}, {"do", TokenKind::KwDo}, {"switch", TokenKind::KwSwitch},
    {"case", TokenKind::KwCase}, {"default", TokenKind::KwDefault}, {"break", TokenKind::KwBreak},
    {"continue", TokenKind::KwContinue}, {"return", TokenKind::KwReturn},
    {"new", TokenKind::KwNew}, {"delete", TokenKind::KwDelete}, {"this", TokenKind::KwThis},
    {"true", TokenKind::KwTrue}, {"false", TokenKind::KwFalse}, {"nullptr", TokenKind::KwNullptr}
};

Lexer::Lexer(std::string source, std::string filename, DiagnosticEngine& diags)
    : source(std::move(source)), filename(std::move(filename)), diags(diags) {
}

Token Lexer::next() {
    if (hasPeeked) {
        hasPeeked = false;
        return currentToken;
    }
    return advanceToken();
}

Token Lexer::peek() {
    if (!hasPeeked) {
        currentToken = advanceToken();
        hasPeeked = true;
    }
    return currentToken;
}

char Lexer::peekChar() const {
    if (isAtEnd()) return '\0';
    return source[offset];
}

char Lexer::nextChar() {
    if (isAtEnd()) return '\0';
    char c = source[offset++];
    if (c == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    return c;
}

bool Lexer::isAtEnd() const {
    return offset >= source.length();
}

bool Lexer::match(char expected) {
    if (isAtEnd() || source[offset] != expected) return false;
    offset++;
    column++;
    return true;
}

SourceLocation Lexer::currentLocation() const {
    return SourceLocation(filename, line, column, offset);
}

Token Lexer::makeToken(TokenKind kind, std::string text) {
    // calculate start position
    int startCol = column - text.length();
    int startOffset = offset - text.length();
    // handles multiline text loosely for now
    return Token(kind, std::move(text), SourceLocation(filename, line, startCol, startOffset));
}

Token Lexer::errorToken(std::string message) {
    SourceLocation loc = currentLocation();
    diags.error(loc, message);
    return Token(TokenKind::Unknown, "", loc);
}

void Lexer::skipWhitespaceAndComments() {
    while (true) {
        char c = peekChar();
        if (std::isspace(c)) {
            nextChar();
        } else if (c == '/') {
            if (offset + 1 < source.length() && source[offset + 1] == '/') {
                // Line comment
                while (!isAtEnd() && peekChar() != '\n') {
                    nextChar();
                }
            } else if (offset + 1 < source.length() && source[offset + 1] == '*') {
                // Block comment
                nextChar(); // '/'
                nextChar(); // '*'
                while (!isAtEnd()) {
                    if (peekChar() == '*' && offset + 1 < source.length() && source[offset + 1] == '/') {
                        nextChar(); // '*'
                        nextChar(); // '/'
                        break;
                    }
                    nextChar();
                }
            } else {
                break; // Just a slash
            }
        } else {
            break;
        }
    }
}

Token Lexer::identifierOrKeyword() {
    SourceLocation startLoc = currentLocation();
    std::string text;
    while (std::isalnum(peekChar()) || peekChar() == '_') {
        text += nextChar();
    }
    auto it = keywords.find(text);
    TokenKind kind = (it != keywords.end()) ? it->second : TokenKind::Identifier;
    return Token(kind, text, startLoc);
}

Token Lexer::number() {
    SourceLocation startLoc = currentLocation();
    std::string text;
    bool isFloat = false;
    
    while (std::isdigit(peekChar())) {
        text += nextChar();
    }
    if (peekChar() == '.' && std::isdigit(source[offset+1])) {
        isFloat = true;
        text += nextChar(); // '.'
        while (std::isdigit(peekChar())) {
            text += nextChar();
        }
    }
    return Token(isFloat ? TokenKind::FloatLiteral : TokenKind::IntegerLiteral, text, startLoc);
}

Token Lexer::advanceToken() {
    skipWhitespaceAndComments();

    if (isAtEnd()) {
        return Token(TokenKind::EndOfFile, "", currentLocation());
    }

    char c = peekChar();
    SourceLocation startLoc = currentLocation();

    if (std::isalpha(c) || c == '_') {
        return identifierOrKeyword();
    }
    if (std::isdigit(c)) {
        return number();
    }

    nextChar(); // consume c

    switch (c) {
        case '+':
            if (match('+')) return Token(TokenKind::Increment, "++", startLoc);
            if (match('=')) return Token(TokenKind::PlusAssign, "+=", startLoc);
            return Token(TokenKind::Plus, "+", startLoc);
        case '-':
            if (match('-')) return Token(TokenKind::Decrement, "--", startLoc);
            if (match('=')) return Token(TokenKind::MinusAssign, "-=", startLoc);
            if (match('>')) return Token(TokenKind::Arrow, "->", startLoc);
            return Token(TokenKind::Minus, "-", startLoc);
        case '*':
            if (match('=')) return Token(TokenKind::StarAssign, "*=", startLoc);
            return Token(TokenKind::Star, "*", startLoc);
        case '/':
            if (match('=')) return Token(TokenKind::SlashAssign, "/=", startLoc);
            return Token(TokenKind::Slash, "/", startLoc);
        case '%':
            return Token(TokenKind::Percent, "%", startLoc);
        case '=':
            if (match('=')) return Token(TokenKind::Equal, "==", startLoc);
            return Token(TokenKind::Assign, "=", startLoc);
        case '!':
            if (match('=')) return Token(TokenKind::NotEqual, "!=", startLoc);
            return Token(TokenKind::LogicalNot, "!", startLoc);
        case '<':
            if (match('=')) return Token(TokenKind::LessEqual, "<=", startLoc);
            if (match('<')) return Token(TokenKind::ShiftLeft, "<<", startLoc);
            return Token(TokenKind::Less, "<", startLoc);
        case '>':
            if (match('=')) return Token(TokenKind::GreaterEqual, ">=", startLoc);
            if (match('>')) return Token(TokenKind::ShiftRight, ">>", startLoc);
            return Token(TokenKind::Greater, ">", startLoc);
        case '&':
            if (match('&')) return Token(TokenKind::LogicalAnd, "&&", startLoc);
            return Token(TokenKind::BitAnd, "&", startLoc);
        case '|':
            if (match('|')) return Token(TokenKind::LogicalOr, "||", startLoc);
            return Token(TokenKind::BitOr, "|", startLoc);
        case '^':
            return Token(TokenKind::BitXor, "^", startLoc);
        case '~':
            return Token(TokenKind::BitNot, "~", startLoc);
        case ':':
            if (match(':')) return Token(TokenKind::ScopeResolution, "::", startLoc);
            return Token(TokenKind::Colon, ":", startLoc);
        case '?': return Token(TokenKind::Question, "?", startLoc);
        case ';': return Token(TokenKind::Semicolon, ";", startLoc);
        case ',': return Token(TokenKind::Comma, ",", startLoc);
        case '#': return Token(TokenKind::Hash, "#", startLoc);
        case '.': return Token(TokenKind::Dot, ".", startLoc);
        case '(': return Token(TokenKind::LParen, "(", startLoc);
        case ')': return Token(TokenKind::RParen, ")", startLoc);
        case '{': return Token(TokenKind::LBrace, "{", startLoc);
        case '}': return Token(TokenKind::RBrace, "}", startLoc);
        case '[': return Token(TokenKind::LBracket, "[", startLoc);
        case ']': return Token(TokenKind::RBracket, "]", startLoc);
        
        // Strings and chars handled later in milestones
        case '"':
        case '\'':
            return errorToken("Strings and chars not fully implemented yet");

        default:
            return errorToken("Unexpected character");
    }
}

} // namespace cppx86
