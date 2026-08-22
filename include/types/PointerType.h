#pragma once
#include "types/Type.h"

namespace cppx86 {

class PointerType : public Type {
public:
    PointerType(TypePtr pointee) : pointee(std::move(pointee)) {}

    TypeKind getKind() const override { return TypeKind::Pointer; }
    
    TypePtr getPointeeType() const { return pointee; }

    bool isPointerType() const override { return true; }

    std::string toString() const override {
        return pointee->toString() + "*";
    }

private:
    TypePtr pointee;
};

class ReferenceType : public Type {
public:
    ReferenceType(TypePtr referee) : referee(std::move(referee)) {}

    TypeKind getKind() const override { return TypeKind::Reference; }
    
    TypePtr getRefereeType() const { return referee; }

    std::string toString() const override {
        return referee->toString() + "&";
    }

private:
    TypePtr referee;
};

} // namespace cppx86
