#pragma once
#include <string>
#include <memory>
#include <vector>

namespace cppx86 {

enum class TypeKind {
    Builtin,
    Pointer,
    Reference,
    Array,
    Function,
    Record // Structs and Classes
};

class Type {
public:
    virtual ~Type() = default;
    
    virtual TypeKind getKind() const = 0;
    virtual std::string toString() const = 0;
    virtual bool isIntegerType() const { return false; }
    virtual bool isFloatingType() const { return false; }
    virtual bool isPointerType() const { return false; }
};

using TypePtr = std::shared_ptr<Type>;

} // namespace cppx86
