#pragma once
#include "types/Type.h"

namespace cppx86 {

class ArrayType : public Type {
public:
    ArrayType(TypePtr elementType, std::size_t size) 
        : elementType(std::move(elementType)), size(size) {}

    TypeKind getKind() const override { return TypeKind::Array; }
    
    TypePtr getElementType() const { return elementType; }
    std::size_t getSize() const { return size; }

    std::string toString() const override {
        return elementType->toString() + "[" + std::to_string(size) + "]";
    }

private:
    TypePtr elementType;
    std::size_t size;
};

} // namespace cppx86
