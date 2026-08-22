#include "ir/IRGenerator.h"
#include "types/BuiltinType.h"

namespace cppx86 {

IRGenerator::IRGenerator() {
    module = std::make_unique<ir::Module>("main_module");
}

std::unique_ptr<ir::Module> IRGenerator::generate(TranslationUnit& tu) {
    tu.accept(*this);
    return std::move(module);
}

void IRGenerator::visit(TranslationUnit& node) {
    for (const auto& decl : node.getDeclarations()) {
        if (auto funcDecl = dynamic_cast<FunctionDecl*>(decl.get())) {
            visit(*funcDecl);
        } else if (auto varDecl = dynamic_cast<VarDecl*>(decl.get())) {
            visit(*varDecl); // Global vars not fully supported yet
        }
    }
}

void IRGenerator::visit(FunctionDecl& node) {
    TypePtr retType = std::make_shared<BuiltinType>(BuiltinTypeKind::Int); // simplified
    
    currentFunction = module->createFunction(retType, node.getName());
    
    if (node.getBody()) {
        ir::BasicBlock* entryBB = currentFunction->createBasicBlock("entry");
        builder.setInsertPoint(entryBB);
        
        namedValues.clear(); // Clear local variables
        
        visit(*node.getBody());
    }
}

void IRGenerator::visit(VarDecl& node) {
    TypePtr type = std::make_shared<BuiltinType>(BuiltinTypeKind::Int); // simplified
    
    // Create alloca
    ir::Instruction* alloca = builder.createAlloca(type, node.getName());
    namedValues[node.getName()] = alloca;
    
    if (node.getInitializer()) {
        visitExpr(node.getInitializer());
        if (lastValue) {
            builder.createStore(lastValue, alloca);
        }
    }
}

void IRGenerator::visit(CompoundStmt& node) {
    for (const auto& stmt : node.getStatements()) {
        if (auto ret = dynamic_cast<ReturnStmt*>(stmt.get())) {
            visit(*ret);
        } else if (auto cmp = dynamic_cast<CompoundStmt*>(stmt.get())) {
            visit(*cmp);
        } else if (auto exp = dynamic_cast<ExprStmt*>(stmt.get())) {
            visit(*exp);
        } else if (auto ds = dynamic_cast<DeclStmt*>(stmt.get())) {
            visit(*ds);
        }
    }
}

void IRGenerator::visit(ReturnStmt& node) {
    if (node.getExpr()) {
        visitExpr(node.getExpr());
        builder.createRet(lastValue);
    } else {
        builder.createRet();
    }
}

void IRGenerator::visit(ExprStmt& node) {
    if (node.getExpr()) {
        visitExpr(node.getExpr());
    }
}

void IRGenerator::visit(DeclStmt& node) {
    if (auto varDecl = dynamic_cast<VarDecl*>(node.getDecl())) {
        visit(*varDecl);
    } else if (auto funcDecl = dynamic_cast<FunctionDecl*>(node.getDecl())) {
        visit(*funcDecl);
    }
}

void IRGenerator::visitExpr(Expr* node) {
    lastValue = nullptr;
    if (auto lit = dynamic_cast<LiteralExpr*>(node)) {
        visit(*lit);
    } else if (auto var = dynamic_cast<VariableExpr*>(node)) {
        visit(*var);
    } else if (auto bin = dynamic_cast<BinaryExpr*>(node)) {
        visit(*bin);
    }
}

void IRGenerator::visit(LiteralExpr& node) {
    // We create a constant value. In a real IR, we'd have a ConstantInt subclass.
    TypePtr type = std::make_shared<BuiltinType>(BuiltinTypeKind::Int);
    lastValue = new ir::Value(type, node.getValue()); // Leaks for now, simplify for milestone
}

void IRGenerator::visit(VariableExpr& node) {
    auto it = namedValues.find(node.getName());
    if (it != namedValues.end()) {
        TypePtr type = std::make_shared<BuiltinType>(BuiltinTypeKind::Int);
        lastValue = builder.createLoad(type, it->second, node.getName() + "_val");
    } else {
        lastValue = nullptr; // Error handling skipped for brevity
    }
}

void IRGenerator::visit(BinaryExpr& node) {
    visitExpr(node.getLHS());
    ir::Value* lhs = lastValue;
    
    visitExpr(node.getRHS());
    ir::Value* rhs = lastValue;
    
    if (!lhs || !rhs) return;
    
    switch (node.getOp()) {
        case TokenKind::Plus:
            lastValue = builder.createAdd(lhs, rhs);
            break;
        case TokenKind::Minus:
            lastValue = builder.createSub(lhs, rhs);
            break;
        // Add more operators as needed
        default:
            lastValue = nullptr;
            break;
    }
}

} // namespace cppx86
