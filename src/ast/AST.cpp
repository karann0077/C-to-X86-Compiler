#include "ast/Declarations/FieldDecl.h"
#include "ast/Declarations/RecordDecl.h"
#include "ast/Expressions/MemberExpr.h"
#include "ast/ASTVisitor.h"

namespace cppx86 {

void FieldDecl::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void RecordDecl::accept(ASTVisitor& visitor) { visitor.visit(*this); }
void MemberExpr::accept(ASTVisitor& visitor) { visitor.visit(*this); }

} // namespace cppx86
