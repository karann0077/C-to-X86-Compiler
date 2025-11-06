#pragma once
#include <string>
#include <vector>
#include <memory>
#include <map>

struct Node {
    virtual ~Node() = default;
};

using NodePtr = std::unique_ptr<Node>;

struct Expr : Node {
    virtual ~Expr() = default;
};

struct Stmt : Node {
    virtual ~Stmt() = default;
};

struct Integer : Expr {
    int value;
    Integer(int v): value(v) {}
};

struct VarExpr : Expr {
    std::string name;
    VarExpr(std::string n): name(std::move(n)) {}
};

struct Binary : Expr {
    std::string op;
    NodePtr lhs, rhs;
    Binary(std::string op_, NodePtr l, NodePtr r): op(op_), lhs(std::move(l)), rhs(std::move(r)) {}
};

struct DeclStmt : Stmt {
    std::string name;
    NodePtr init; // may be null
    DeclStmt(std::string n, NodePtr i): name(std::move(n)), init(std::move(i)) {}
};

struct ExprStmt : Stmt {
    NodePtr expr;
    ExprStmt(NodePtr e): expr(std::move(e)) {}
};

struct ReturnStmt : Stmt {
    NodePtr expr;
    ReturnStmt(NodePtr e): expr(std::move(e)) {}
};

struct IfStmt : Stmt {
    NodePtr cond;
    NodePtr thenStmt;
    NodePtr elseStmt; // may be null
    IfStmt(NodePtr c, NodePtr t, NodePtr e): cond(std::move(c)), thenStmt(std::move(t)), elseStmt(std::move(e)) {}
};

struct WhileStmt : Stmt {
    NodePtr cond;
    NodePtr body;
    WhileStmt(NodePtr c, NodePtr b): cond(std::move(c)), body(std::move(b)) {}
};

struct BlockStmt : Stmt {
    std::vector<NodePtr> stmts;
};

struct Function {
    std::string name;
    std::vector<NodePtr> body; // list of Stmt
};

struct Program {
    std::vector<Function> funcs;
};
