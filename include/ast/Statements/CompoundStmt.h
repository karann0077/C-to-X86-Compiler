#pragma once
#include "ast/Stmt.h"
#include <vector>

namespace cppx86 {

class CompoundStmt : public Stmt {
public:
    void addStatement(StmtPtr stmt) {
        statements.push_back(std::move(stmt));
    }
    
    void accept(ASTVisitor& visitor) override {}

    const std::vector<StmtPtr>& getStatements() const { return statements; }

private:
    std::vector<StmtPtr> statements;
};

} // namespace cppx86
