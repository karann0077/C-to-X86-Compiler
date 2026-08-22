#pragma once
#include "common/SourceLocation.h"
#include <string>
#include <vector>

namespace cppx86 {

enum class TokenKind {
    // Basic types
    Identifier,
    IntegerLiteral,
    FloatLiteral,
    StringLiteral,
    CharLiteral,

    // Keywords
    KwInt, KwLong, KwShort, KwChar, KwFloat, KwDouble, KwVoid, KwBool,
    KwAuto, KwConst, KwStatic, KwExtern, KwClass, KwStruct,
    KwPublic, KwPrivate, KwProtected, KwVirtual, KwOverride,
    KwTemplate, KwTypename, KwUsing, KwNamespace,
    KwIf, KwElse, KwWhile, KwFor, KwDo, KwSwitch, KwCase, KwDefault,
    KwBreak, KwContinue, KwReturn, KwNew, KwDelete, KwThis,
    KwTrue, KwFalse, KwNullptr,

    // Operators
    Plus, Minus, Star, Slash, Percent,
    Equal, NotEqual, Less, LessEqual, Greater, GreaterEqual,
    LogicalAnd, LogicalOr, LogicalNot,
    BitAnd, BitOr, BitXor, BitNot,
    ShiftLeft, ShiftRight,
    Assign, PlusAssign, MinusAssign, StarAssign, SlashAssign,
    Increment, Decrement,
    Question, Colon, Semicolon, Comma, Hash,

    // Brackets
    LParen, RParen, LBrace, RBrace, LBracket, RBracket,

    // Access
    Dot, Arrow, ScopeResolution,

    EndOfFile,
    Unknown
};

struct Token {
    TokenKind kind;
    std::string text;
    SourceLocation location;

    Token() : kind(TokenKind::Unknown) {}
    Token(TokenKind kind, std::string text, SourceLocation loc)
        : kind(kind), text(std::move(text)), location(std::move(loc)) {}
};

class TokenStream {
public:
    virtual ~TokenStream() = default;
    virtual Token next() = 0;
    virtual Token peek() = 0;
};

} // namespace cppx86
