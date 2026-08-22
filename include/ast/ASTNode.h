#pragma once
#include "common/SourceLocation.h"

#include "ast/ASTVisitor.h"

namespace cppx86 {

class ASTNode {
public:
    virtual ~ASTNode() = default;
    
    virtual void accept(ASTVisitor& visitor) = 0;

    SourceLocation getLocation() const { return location; }
    void setLocation(SourceLocation loc) { location = std::move(loc); }

private:
    SourceLocation location;
};

} // namespace cppx86
