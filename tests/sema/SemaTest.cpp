#include <gtest/gtest.h>
#include "lexer/Lexer.h"
#include "parser/Parser.h"
#include "sema/SemanticAnalyzer.h"
#include "diagnostics/DiagnosticEngine.h"

using namespace cppx86;

TEST(SemaTest, UndeclaredVar) {
    DiagnosticEngine diags;
    Lexer lexer("int main() { return y; }", "test.cpp", diags);
    Parser parser(lexer, diags);
    auto tu = parser.parse();
    EXPECT_FALSE(diags.hasErrors());

    SemanticAnalyzer sema(diags);
    sema.analyze(*tu);
    EXPECT_TRUE(diags.hasErrors());
}
