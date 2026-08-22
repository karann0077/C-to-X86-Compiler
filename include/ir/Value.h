#pragma once
#include "types/Type.h"
#include <string>

namespace cppx86 {
namespace ir {

class Value {
public:
    Value(TypePtr type, std::string name = "") 
        : type(std::move(type)), name(std::move(name)) {}
    virtual ~Value() = default;

    TypePtr getType() const { return type; }
    const std::string& getName() const { return name; }
    void setName(std::string n) { name = std::move(n); }

    virtual std::string toString() const {
        if (!name.empty()) return "%" + name;
        return "%<unnamed>";
    }

private:
    TypePtr type;
    std::string name;
};

} // namespace ir
} // namespace cppx86
