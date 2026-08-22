#pragma once
#include "ir/Module.h"

namespace cppx86 {
namespace opt {

class Pass {
public:
    virtual ~Pass() = default;
    virtual bool runOnModule(ir::Module& module) = 0;
};

} // namespace opt
} // namespace cppx86
