#pragma once
#include "ast/Decl.h"
#include "ast/Declarations/FieldDecl.h"
#include "ast/Declarations/FunctionDecl.h"
#include <vector>

namespace cppx86 {

class RecordDecl : public Decl {
public:
    RecordDecl(std::string name, bool isClass) 
        : Decl(std::move(name)), isClass(isClass) {}

    void accept(ASTVisitor& visitor) override;

    void addField(FieldDeclPtr field) { fields.push_back(std::move(field)); }
    void addMethod(FunctionDeclPtr method) { methods.push_back(std::move(method)); }

    const std::vector<FieldDeclPtr>& getFields() const { return fields; }
    const std::vector<FunctionDeclPtr>& getMethods() const { return methods; }
    
    bool getIsClass() const { return isClass; }

private:
    bool isClass; // true for class, false for struct
    std::vector<FieldDeclPtr> fields;
    std::vector<FunctionDeclPtr> methods;
};

using RecordDeclPtr = std::unique_ptr<RecordDecl>;

} // namespace cppx86
