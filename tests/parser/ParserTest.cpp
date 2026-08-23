#include <gtest/gtest.h>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "diagnostics/DiagnosticEngine.h"

using namespace cppx86;

TEST(ParserTest, BasicVarDecl) {
    DiagnosticEngine diags;
    Lexer lexer("int x = 10;", "test.cpp", diags);
    Parser parser(lexer, diags);
    auto tu = parser.parse();
    EXPECT_FALSE(diags.hasErrors());
    EXPECT_EQ(tu->getDeclarations().size(), 1);
}
