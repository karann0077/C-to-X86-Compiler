#pragma once
#include "ast/Decl.h"
#include "ast/TypeNode.h"
#include "ast/Statements/CompoundStmt.h"
#include <vector>

namespace cppx86 {

struct Parameter {
    std::string name;
    TypeNodePtr type;
};

class FunctionDecl : public Decl {
public:
    FunctionDecl(std::string name, TypeNodePtr returnType) 
        : Decl(std::move(name)), returnType(std::move(returnType)) {}

    void addParameter(std::string name, TypeNodePtr type) {
        parameters.push_back({std::move(name), std::move(type)});
    }

    void setBody(std::unique_ptr<CompoundStmt> b) { body = std::move(b); }
    
    void accept(ASTVisitor& visitor) override {}

private:
    TypeNodePtr returnType;
    std::vector<Parameter> parameters;
    std::unique_ptr<CompoundStmt> body; // null if just a declaration
};

} // namespace cppx86
