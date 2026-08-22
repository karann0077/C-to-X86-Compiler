#pragma once
#include "types/Type.h"
#include <vector>

namespace cppx86 {

class FunctionType : public Type {
public:
    FunctionType(TypePtr returnType, std::vector<TypePtr> paramTypes) 
        : returnType(std::move(returnType)), paramTypes(std::move(paramTypes)) {}

    TypeKind getKind() const override { return TypeKind::Function; }
    
    TypePtr getReturnType() const { return returnType; }
    const std::vector<TypePtr>& getParamTypes() const { return paramTypes; }

    std::string toString() const override {
        std::string result = returnType->toString() + "(";
        for (std::size_t i = 0; i < paramTypes.size(); ++i) {
            result += paramTypes[i]->toString();
            if (i + 1 < paramTypes.size()) {
                result += ", ";
            }
        }
        result += ")";
        return result;
    }

private:
    TypePtr returnType;
    std::vector<TypePtr> paramTypes;
};

} // namespace cppx86
