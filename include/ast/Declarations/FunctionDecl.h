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
        : Decl(std::move(name)), returnType(std::move(returnType)), isMethod(false) {}

    void addParameter(std::string name, TypeNodePtr type) {
        parameters.push_back({std::move(name), std::move(type)});
    }

    const std::vector<Parameter>& getParameters() const { return parameters; }
    
    bool getIsMethod() const { return isMethod; }
    void setIsMethod(bool b) { isMethod = b; }

    void setBody(std::unique_ptr<CompoundStmt> b) { body = std::move(b); }
    
    CompoundStmt* getBody() const { return body.get(); }
    
    void accept(ASTVisitor& visitor) override {}

private:
    TypeNodePtr returnType;
    std::vector<Parameter> parameters;
    std::unique_ptr<CompoundStmt> body; // null if just a declaration
    bool isMethod;
};

using FunctionDeclPtr = std::unique_ptr<FunctionDecl>;

} // namespace cppx86
