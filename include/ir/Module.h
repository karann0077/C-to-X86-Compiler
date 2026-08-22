#pragma once
#include "ir/Function.h"
#include <vector>
#include <memory>
#include <string>
#include <iostream>

namespace cppx86 {
namespace ir {

class Module {
public:
    Module(std::string name) : name(std::move(name)) {}

    Function* createFunction(TypePtr type, const std::string& name) {
        auto func = std::make_unique<Function>(std::move(type), name, this);
        Function* ptr = func.get();
        functions.push_back(std::move(func));
        return ptr;
    }

    const std::string& getName() const { return name; }
    
    const std::vector<std::unique_ptr<Function>>& getFunctions() const {
        return functions;
    }
    
    void dump(std::ostream& os) const {
        os << "; Module: " << name << "\n\n";
        for (const auto& func : functions) {
            os << "define " << func->getType()->toString() << " " << func->toString() << "() {\n";
            for (const auto& bb : func->getBlocks()) {
                os << bb->toString() << "\n";
                for (const auto& inst : bb->getInstructions()) {
                    os << "  " << inst->print() << "\n";
                }
            }
            os << "}\n\n";
        }
    }

private:
    std::string name;
    std::vector<std::unique_ptr<Function>> functions;
};

} // namespace ir
} // namespace cppx86
