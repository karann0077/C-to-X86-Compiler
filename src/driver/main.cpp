#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "lexer/Lexer.h"
#include "diagnostics/DiagnosticEngine.h"
#include "parser/Parser.h"
#include "ast/ASTPrinter.h"
#include "sema/SemanticAnalyzer.h"

using namespace cppx86;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: cppx86 [options] file...\n";
        return 1;
    }

    std::string filename;
    bool dumpTokens = false;
    bool dumpAst = false;
    bool dumpIr = false;
    bool dumpAsm = false;
    bool dumpSema = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-tokens") {
            dumpTokens = true;
        } else if (arg == "-ast") {
            dumpAst = true;
        } else if (arg == "-sema") {
            dumpSema = true;
        } else if (arg == "-ir") {
            dumpIr = true;
        } else if (arg == "-asm") {
            dumpAsm = true;
        } else {
            filename = arg;
        }
    }

    if (filename.empty()) {
        std::cerr << "Error: no input files\n";
        return 1;
    }

    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error: cannot open file " << filename << "\n";
        return 1;
    }

    std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    
    DiagnosticEngine diags;

    if (dumpTokens) {
        Lexer lexer(source, filename, diags);
        while (true) {
            Token t = lexer.next();
            std::cout << "Token: " << t.text << " @ " << t.location.line << ":" << t.location.column << "\n";
            if (t.kind == TokenKind::EndOfFile) {
                break;
            }
        }
    } else if (dumpAst) {
        Lexer lexer(source, filename, diags);
        Parser parser(lexer, diags);
        auto tu = parser.parse();
        
        if (!diags.hasErrors()) {
            ASTPrinter printer;
            printer.print(*tu);
        }
    } else if (dumpSema) {
        Lexer lexer(source, filename, diags);
        Parser parser(lexer, diags);
        auto tu = parser.parse();
        
        if (!diags.hasErrors()) {
            SemanticAnalyzer sema(diags);
            sema.analyze(*tu);
            if (!diags.hasErrors()) {
                std::cout << "Semantic analysis completed successfully.\n";
            }
        }
    } else {
        // Full pipeline would go here
    }

    if (diags.hasErrors()) {
        return 1;
    }

    return 0;
}
