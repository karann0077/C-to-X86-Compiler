#include <gtest/gtest.h>
#include "lexer/Lexer.h"
#include "diagnostics/DiagnosticEngine.h"

using namespace cppx86;

TEST(LexerTest, BasicTokens) {
    DiagnosticEngine diags;
    Lexer lexer("int x = 10; // comment\n", "test.cpp", diags);

    Token t = lexer.next();
    EXPECT_EQ(t.kind, TokenKind::KwInt);
    EXPECT_EQ(t.text, "int");

    t = lexer.next();
    EXPECT_EQ(t.kind, TokenKind::Identifier);
    EXPECT_EQ(t.text, "x");

    t = lexer.next();
    EXPECT_EQ(t.kind, TokenKind::Assign);
    EXPECT_EQ(t.text, "=");

    t = lexer.next();
    EXPECT_EQ(t.kind, TokenKind::IntegerLiteral);
    EXPECT_EQ(t.text, "10");

    t = lexer.next();
    EXPECT_EQ(t.kind, TokenKind::Semicolon);
    EXPECT_EQ(t.text, ";");

    t = lexer.next();
    EXPECT_EQ(t.kind, TokenKind::EndOfFile);
    
    EXPECT_FALSE(diags.hasErrors());
}
