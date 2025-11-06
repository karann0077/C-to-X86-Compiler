#pragma once
#include "ast.h"
#include <string>

class CodeGen {
public:
    CodeGen(const Program &p);
    std::string generate(); // returns assembly as string
private:
    const Program &prog;
    int labelCounter = 0;
    std::string emitLabel();
    std::string emitFunction(const Function &f);
    // simple symbol table per function: name -> stack offset
    void assignStackOffsets(const Function &f, std::map<std::string,int> &offsets, int &frameSize);
    std::string genStmt(Node *n, std::map<std::string,int> &offsets);
    std::string genExpr(Node *n, std::map<std::string,int> &offsets);
};
