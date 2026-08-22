#pragma once
#include "opt/Pass.h"
#include <vector>
#include <memory>

namespace cppx86 {
namespace opt {

class PassManager {
public:
    void addPass(std::unique_ptr<Pass> pass) {
        passes.push_back(std::move(pass));
    }

    void run(ir::Module& module) {
        bool changed;
        do {
            changed = false;
            for (auto& pass : passes) {
                changed |= pass->runOnModule(module);
            }
        } while (changed); // Run until fixpoint
    }

private:
    std::vector<std::unique_ptr<Pass>> passes;
};

} // namespace opt
} // namespace cppx86
