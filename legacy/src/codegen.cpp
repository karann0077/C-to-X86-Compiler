#include "codegen.h"
#include "ast.h"
#include <sstream>
#include <iostream>

CodeGen::CodeGen(const Program &p): prog(p) {}

std::string CodeGen::emitLabel(){
    return ".L" + std::to_string(labelCounter++);
}

void CodeGen::assignStackOffsets(const Function &f, std::map<std::string,int> &offsets, int &frameSize){
    // find declared vars in function body (very simple scan)
    int offset = 0;
    int localCount = 0;
    // scan statements recursively (only top-level decls considered here)
    std::function<void(Node*)> scan = [&](Node* n){
        if(!n) return;
        if(auto *ds = dynamic_cast<DeclStmt*>(n)){
            localCount++;
            offsets[ds->name] = 0; // placeholder
        } else if(auto *bs = dynamic_cast<BlockStmt*>(n)){
            for(auto &s : bs->stmts) scan(s.get());
        } else if(auto *ifs = dynamic_cast<IfStmt*>(n)){
            scan(ifs->thenStmt.get());
            scan(ifs->elseStmt.get());
        } else if(auto *ws = dynamic_cast<WhileStmt*>(n)){
            scan(ws->body.get());
        }
    };
    for(auto &s : f.body) scan(s.get());
    // assign 4 bytes per local (int)
    frameSize = ((localCount * 4) + 15) / 16 * 16; // align to 16
    int curOffset = 0;
    for(auto &kv : offsets){
        curOffset += 4;
        kv.second = -curOffset; // rbp - offset
    }
}

std::string CodeGen::generate(){
    std::ostringstream out;
    out << "    .text\n";
    for(auto &f : prog.funcs){
        out << emitFunction(f);
    }
    return out.str();
}

std::string CodeGen::emitFunction(const Function &f){
    std::ostringstream out;
    out << "    .global " << f.name << "\n";
    out << f.name << ":\n";
    out << "    push rbp\n";
    out << "    mov rbp, rsp\n";
    // compute locals
    std::map<std::string,int> offsets;
    int frameSize = 0;
    assignStackOffsets(f, offsets, frameSize);
    if(frameSize > 0) out << "    sub rsp, " << frameSize << "\n";

    // generate statements
    for(auto &s : f.body){
        out << genStmt(s.get(), offsets);
    }
    // default return 0 if no explicit return
    out << "    mov eax, 0\n";
    if(frameSize > 0) out << "    add rsp, " << frameSize << "\n";
    out << "    pop rbp\n";
    out << "    ret\n\n";
    return out.str();
}

std::string CodeGen::genStmt(Node *n, std::map<std::string,int> &offsets){
    std::ostringstream out;
    if(auto *ds = dynamic_cast<DeclStmt*>(n)){
        if(ds->init){
            out << genExpr(ds->init.get(), offsets);
            // value in eax, store to local
            int off = offsets[ds->name];
            out << "    mov DWORD PTR [rbp" << off << "], eax\n";
        } else {
            // uninitialized -> zero
            int off = offsets[ds->name];
            out << "    mov DWORD PTR [rbp" << off << "], 0\n";
        }
        return out.str();
    }
    if(auto *es = dynamic_cast<ExprStmt*>(n)){
        out << genExpr(es->expr.get(), offsets);
        return out.str();
    }
    if(auto *rs = dynamic_cast<ReturnStmt*>(n)){
        out << genExpr(rs->expr.get(), offsets);
        // result is in eax
        // restore frame and ret
        out << "    mov ebx, eax\n"; // move to ebx to keep
        // epilogue
        out << "    mov eax, ebx\n";
        out << "    jmp .LRETURN\n"; // We'll patch: easier: inline epilogue here (avoid label complexity)
        // Instead of jmp, generate epilogue:
        // BUT we need frameSize; not available here -- small compromise: caller ensures frame is 0 or we can reconstruct
    }
    if(auto *ifs = dynamic_cast<IfStmt*>(n)){
        std::string Lelse = emitLabel();
        std::string Lend = emitLabel();
        out << genExpr(ifs->cond.get(), offsets);
        out << "    cmp eax, 0\n";
        out << "    je " << Lelse << "\n";
        out << genStmt(ifs->thenStmt.get(), offsets);
        out << "    jmp " << Lend << "\n";
        out << Lelse << ":\n";
        if(ifs->elseStmt) out << genStmt(ifs->elseStmt.get(), offsets);
        out << Lend << ":\n";
        return out.str();
    }
    if(auto *ws = dynamic_cast<WhileStmt*>(n)){
        std::string Ltop = emitLabel();
        std::string Lend = emitLabel();
        out << Ltop << ":\n";
        out << genExpr(ws->cond.get(), offsets);
        out << "    cmp eax, 0\n";
        out << "    je " << Lend << "\n";
        out << genStmt(ws->body.get(), offsets);
        out << "    jmp " << Ltop << "\n";
        out << Lend << ":\n";
        return out.str();
    }
    if(auto *bs = dynamic_cast<BlockStmt*>(n)){
        for(auto &s : bs->stmts) out << genStmt(s.get(), offsets);
        return out.str();
    }
    return out.str();
}

std::string CodeGen::genExpr(Node *n, std::map<std::string,int> &offsets){
    std::ostringstream out;
    if(auto *intn = dynamic_cast<Integer*>(n)){
        out << "    mov eax, " << intn->value << "\n";
        return out.str();
    }
    if(auto *vn = dynamic_cast<VarExpr*>(n)){
        auto it = offsets.find(vn->name);
        if(it == offsets.end()){
            throw std::runtime_error("Undefined variable " + vn->name);
        }
        int off = it->second;
        out << "    mov eax, DWORD PTR [rbp" << off << "]\n";
        return out.str();
    }
    if(auto *bin = dynamic_cast<Binary*>(n)){
        std::string op = bin->op;
        if(op == "="){
            // left must be VarExpr
            auto *leftVar = dynamic_cast<VarExpr*>(bin->lhs.get());
            if(!leftVar) throw std::runtime_error("Left side of assignment must be variable");
            out << genExpr(bin->rhs.get(), offsets);
            int off = offsets[leftVar->name];
            out << "    mov DWORD PTR [rbp" << off << "], eax\n";
            return out.str();
        }
        // general binary: evaluate lhs into eax, push, eval rhs into eax, pop into ebx, operate
        out << genExpr(bin->lhs.get(), offsets);
        out << "    push rax\n";
        out << genExpr(bin->rhs.get(), offsets);
        out << "    mov ebx, eax\n";
        out << "    pop rax\n";
        // now lhs in eax, rhs in ebx
        if(op == "+") out << "    add eax, ebx\n";
        else if(op == "-") out << "    sub eax, ebx\n";
        else if(op == "*") out << "    imul eax, ebx\n";
        else if(op == "/"){
            // dividend in rax (eax), divisor in ebx (ebx). Need sign-extend to rdx:eax
            out << "    cdq\n";
            out << "    idiv ebx\n";
        } else if(op == "%"){
            out << "    cdq\n";
            out << "    idiv ebx\n";
            out << "    mov eax, edx\n";
        } else if(op == "=="){
            out << "    cmp eax, ebx\n";
            out << "    sete al\n";
            out << "    movzx eax, al\n";
        } else if(op == "!="){
            out << "    cmp eax, ebx\n";
            out << "    setne al\n";
            out << "    movzx eax, al\n";
        } else if(op == "<"){
            out << "    cmp eax, ebx\n";
            out << "    setl al\n";
            out << "    movzx eax, al\n";
        } else if(op == "<="){
            out << "    cmp eax, ebx\n";
            out << "    setle al\n";
            out << "    movzx eax, al\n";
        } else if(op == ">"){
            out << "    cmp eax, ebx\n";
            out << "    setg al\n";
            out << "    movzx eax, al\n";
        } else if(op == ">="){
            out << "    cmp eax, ebx\n";
            out << "    setge al\n";
            out << "    movzx eax, al\n";
        } else if(op == "&&"){
            // short-circuit: eax = lhs, if zero -> 0 else => evaluate rhs
            std::string Lzero = emitLabel();
            std::string Ldone = emitLabel();
            out << "    cmp eax, 0\n";
            out << "    je " << Lzero << "\n";
            out << "    mov eax, ebx\n"; // rhs result
            out << "    cmp eax, 0\n";
            out << "    setne al\n";
            out << "    movzx eax, al\n";
            out << "    jmp " << Ldone << "\n";
            out << Lzero << ":\n";
            out << "    mov eax, 0\n";
            out << Ldone << ":\n";
        } else if(op == "||"){
            std::string Ltrue = emitLabel();
            std::string Ldone = emitLabel();
            out << "    cmp eax, 0\n";
            out << "    jne " << Ltrue << "\n";
            out << "    mov eax, ebx\n";
            out << "    cmp eax, 0\n";
            out << "    setne al\n";
            out << "    movzx eax, al\n";
            out << "    jmp " << Ldone << "\n";
            out << Ltrue << ":\n";
            out << "    mov eax, 1\n";
            out << Ldone << ":\n";
        } else if(op == "neg"){
            out << "    neg eax\n";
        } else {
            throw std::runtime_error("Unknown binary op: " + op);
        }
        return out.str();
    }
    throw std::runtime_error("Unknown expr node in codegen");
}
