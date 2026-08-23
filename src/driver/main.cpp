#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "lexer/Lexer.h"
#include "diagnostics/DiagnosticEngine.h"
#include "parser/Parser.h"
#include "ast/ASTPrinter.h"
#include "sema/SemanticAnalyzer.h"
#include "ir/IRGenerator.h"
#include "codegen/X86Backend.h"
#include "opt/PassManager.h"
#include "opt/ConstantFoldingPass.h"
#include "opt/DeadCodeEliminationPass.h"

using namespace cppx86;

#include <vector>
#include <cstdlib>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [options] <files...>\n";
        return 1;
    }

    bool dumpTokens = false;
    bool dumpAst = false;
    bool dumpSema = false;
    bool dumpIr = false;
    bool dumpOpt = false;
    bool dumpAsm = false;
    bool compileOnly = false;
    std::string outputFile = "a.out";
    std::vector<std::string> inputFiles;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-tokens") dumpTokens = true;
        else if (arg == "-ast") dumpAst = true;
        else if (arg == "-sema") dumpSema = true;
        else if (arg == "-ir") dumpIr = true;
        else if (arg == "-opt") dumpOpt = true;
        else if (arg == "-asm") dumpAsm = true;
        else if (arg == "-c") compileOnly = true;
        else if (arg == "-o" && i + 1 < argc) {
            outputFile = argv[++i];
        }
        else {
            inputFiles.push_back(arg);
        }
    }

    if (inputFiles.empty()) {
        std::cerr << "Error: no input files\n";
        return 1;
    }

    DiagnosticEngine diags;
    bool hadErrors = false;
    std::vector<std::string> asmFiles;

    for (const auto& filename : inputFiles) {
        std::ifstream file(filename);
        if (!file) {
            std::cerr << "Error: cannot open file " << filename << "\n";
            hadErrors = true;
            continue;
        }

        std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        diags.setSource(source);

        Lexer lexer(source, filename, diags);
        
        if (dumpTokens) {
            while (true) {
                Token t = lexer.next();
                std::cout << "Token: " << t.text << " @ " << t.location.line << ":" << t.location.column << "\n";
                if (t.kind == TokenKind::EndOfFile) break;
            }
            continue;
        }

        Parser parser(lexer, diags);
        auto tu = parser.parse();

        if (diags.hasErrors()) { hadErrors = true; continue; }

        if (dumpAst) {
            ASTPrinter printer;
            printer.print(*tu);
            continue;
        }

        SemanticAnalyzer sema(diags);
        sema.analyze(*tu);

        if (diags.hasErrors()) { hadErrors = true; continue; }

        if (dumpSema) {
            std::cout << "Semantic analysis for " << filename << " completed successfully.\n";
            continue;
        }

        IRGenerator irGen;
        auto module = irGen.generate(*tu);

        opt::PassManager pm;
        pm.addPass(std::make_unique<opt::ConstantFoldingPass>());
        pm.addPass(std::make_unique<opt::DeadCodeEliminationPass>());
        pm.run(*module);

        if (dumpOpt) {
            module->dump(std::cout);
            continue;
        } else if (dumpIr) {
            module->dump(std::cout);
            continue;
        }

        // Generate Assembly
        std::string asmFilename = filename.substr(0, filename.find_last_of('.')) + ".s";
        std::ofstream asmFile(asmFilename);
        if (!asmFile) {
            std::cerr << "Error: cannot write to " << asmFilename << "\n";
            hadErrors = true;
            continue;
        }
        
        codegen::X86Backend backend;
        backend.generate(*module, asmFile);
        asmFile.close();
        
        if (dumpAsm) {
            codegen::X86Backend consoleBackend;
            consoleBackend.generate(*module, std::cout);
        }

        asmFiles.push_back(asmFilename);
    }

    if (hadErrors) {
        return 1;
    }

    if (dumpTokens || dumpAst || dumpSema || dumpIr || dumpOpt || dumpAsm || compileOnly) {
        return 0; // Stop if we're just dumping or compiling
    }

    // Linking Phase
    std::string gccCmd = "gcc ";
    for (const auto& asmFile : asmFiles) {
        gccCmd += asmFile + " ";
    }
    gccCmd += "-o " + outputFile;

    int ret = std::system(gccCmd.c_str());
    if (ret != 0) {
        std::cerr << "Error: linking failed\n";
        return 1;
    }

    return 0;
}
