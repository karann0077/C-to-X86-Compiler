#include "lexer.h"
#include "parser.h"
#include "codegen.h"
#include <fstream>
#include <iostream>

int main(int argc, char **argv){
    if(argc < 2){
        std::cerr << "Usage: tinycc <source.tc>\n";
        return 1;
    }
    std::ifstream in(argv[1]);
    if(!in){ std::cerr << "Cannot open file\n"; return 1; }
    std::string src((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

    try {
        Lexer lx(src);
        Parser p(lx);
        Program prog = p.parse();
        CodeGen cg(prog);
        std::string asmcode = cg.generate();
        std::string outAsm = std::string(argv[1]) + ".s";
        std::ofstream out(outAsm);
        out << asmcode;
        out.close();
        std::cout << "Assembly written to " << outAsm << "\n";
        std::cout << "Now assemble & link with: gcc -no-pie -o prog " << outAsm << "\n";
    } catch(std::exception &e){
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
