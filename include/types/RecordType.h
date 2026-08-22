#pragma once
#include "types/Type.h"
#include <string>

namespace cppx86 {

class RecordType : public Type {
public:
    RecordType(std::string name, bool isStruct) 
        : name(std::move(name)), isStruct(isStruct) {}

    TypeKind getKind() const override { return TypeKind::Record; }
    
    const std::string& getName() const { return name; }
    bool getIsStruct() const { return isStruct; }

    std::string toString() const override {
        return (isStruct ? "struct " : "class ") + name;
    }

private:
    std::string name;
    bool isStruct;
};

} // namespace cppx86
