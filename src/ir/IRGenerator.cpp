#include "ir/IRGenerator.h"
#include "types/BuiltinType.h"
#include "ast/Declarations/RecordDecl.h"
#include "ast/Declarations/FieldDecl.h"
#include "ast/Expressions/MemberExpr.h"
#include "ast/Statements/IfStmt.h"
#include "ast/Statements/WhileStmt.h"

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
        } else if (auto recDecl = dynamic_cast<RecordDecl*>(decl.get())) {
            visit(*recDecl);
        }
    }
}

void IRGenerator::visit(RecordDecl& node) {
    // In IR, a class is just a struct definition mapping fields to offsets
    // For now, we'll skip generating full type layouts in the simplistic IR.
    for (const auto& method : node.getMethods()) {
        visit(*method);
    }
}

void IRGenerator::visit(FunctionDecl& node) {
    TypePtr retType = std::make_shared<BuiltinType>(BuiltinTypeKind::Int); // simplified
    
    currentFunction = module->createFunction(retType, node.getName());
    
    if (node.getBody()) {
        ir::BasicBlock* entryBB = currentFunction->createBasicBlock("entry");
        builder.setInsertPoint(entryBB);
        
        namedValues.clear(); // Clear local variables
        
        if (node.getIsMethod()) {
            TypePtr ptrType = std::make_shared<BuiltinType>(BuiltinTypeKind::Int);
            ir::Value* thisVal = builder.createAlloca(ptrType, "this");
            namedValues["this"] = thisVal;
        }
        
        visit(*node.getBody());
    }
}

void IRGenerator::visit(FieldDecl& node) {
    // Fields don't generate instructions on their own, they are part of Record layout
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
        stmt->accept(*this);
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

void IRGenerator::visit(IfStmt& node) {
    ir::BasicBlock* thenBB = currentFunction->createBasicBlock("if.then");
    ir::BasicBlock* elseBB = node.getElseBlock() ? currentFunction->createBasicBlock("if.else") : nullptr;
    ir::BasicBlock* mergeBB = currentFunction->createBasicBlock("if.end");

    // Evaluate condition
    visitExpr(node.getCondition());
    ir::Value* condVal = lastValue;

    // Branch
    builder.createCondBr(condVal, thenBB, elseBB ? elseBB : mergeBB);

    // Then block
    builder.setInsertPoint(thenBB);
    node.getThenBlock()->accept(*this);
    builder.createBr(mergeBB); // Merge

    // Else block
    if (elseBB) {
        builder.setInsertPoint(elseBB);
        node.getElseBlock()->accept(*this);
        builder.createBr(mergeBB); // Merge
    }

    // Continue at merge block
    builder.setInsertPoint(mergeBB);
}

void IRGenerator::visit(WhileStmt& node) {
    ir::BasicBlock* condBB = currentFunction->createBasicBlock("while.cond");
    ir::BasicBlock* bodyBB = currentFunction->createBasicBlock("while.body");
    ir::BasicBlock* endBB = currentFunction->createBasicBlock("while.end");

    // Jump to condition block
    builder.createBr(condBB);

    // Condition block
    builder.setInsertPoint(condBB);
    visitExpr(node.getCondition());
    ir::Value* condVal = lastValue;
    builder.createCondBr(condVal, bodyBB, endBB);

    // Body block
    builder.setInsertPoint(bodyBB);
    node.getBody()->accept(*this);
    builder.createBr(condBB); // loop back

    // End block
    builder.setInsertPoint(endBB);
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
    } else if (auto mem = dynamic_cast<MemberExpr*>(node)) {
        visit(*mem);
    }
}

void IRGenerator::visit(MemberExpr& node) {
    // For a MemberExpr `obj.field`, we evaluate `obj` which gives us a pointer (alloca)
    // Then we would emit a GetElementPtr (GEP) to add the field offset.
    // For this simple milestone, we'll pretend there's only one field or use a fake GEP.
    visitExpr(node.getBase());
    ir::Value* baseVal = lastValue;
    
    // We emit a fake offset addition since we don't have struct types fully fleshed out in IR yet.
    TypePtr type = std::make_shared<BuiltinType>(BuiltinTypeKind::Int);
    
    // Create a dummy add for the pointer offset (assuming 4 bytes per field)
    // Normally this is what a GEP instruction does.
    ir::Value* offset = new ir::Value(type, "4"); // Just hardcoding offset 4 for demo
    lastValue = builder.createAdd(baseVal, offset);
}

void IRGenerator::visit(LiteralExpr& node) {
    // We create a constant value. In a real IR, we'd have a ConstantInt subclass.
    TypePtr type = std::make_shared<BuiltinType>(BuiltinTypeKind::Int);
    lastValue = new ir::Value(type, node.getValue()); // Leaks for now, simplify for milestone
}

void IRGenerator::visit(VariableExpr& node) {
    if (node.getIsField()) {
        auto it = namedValues.find("this");
        if (it != namedValues.end()) {
            TypePtr type = std::make_shared<BuiltinType>(BuiltinTypeKind::Int);
            ir::Value* thisPtr = builder.createLoad(type, it->second, "this_val");
            ir::Value* offset = new ir::Value(type, "4"); // dummy offset
            ir::Value* fieldPtr = builder.createAdd(thisPtr, offset);
            lastValue = builder.createLoad(type, fieldPtr, node.getName() + "_val");
        } else {
            lastValue = nullptr;
        }
        return;
    }

    auto it = namedValues.find(node.getName());
    if (it != namedValues.end()) {
        TypePtr type = std::make_shared<BuiltinType>(BuiltinTypeKind::Int);
        lastValue = builder.createLoad(type, it->second, node.getName() + "_val");
    } else {
        lastValue = nullptr; // Error handling skipped for brevity
    }
}

void IRGenerator::visit(BinaryExpr& node) {
    if (node.getOp() == TokenKind::Assign) {
        // Assignment requires an L-value on the left side.
        // For simplicity, we just look up the named value directly instead of full L-value evaluation.
        ir::Value* lhsPtr = nullptr;
        if (auto varExpr = dynamic_cast<VariableExpr*>(node.getLHS())) {
            auto it = namedValues.find(varExpr->getName());
            if (it != namedValues.end()) lhsPtr = it->second;
        }
        
        visitExpr(node.getRHS());
        ir::Value* rhs = lastValue;
        
        if (lhsPtr && rhs) {
            builder.createStore(rhs, lhsPtr);
            lastValue = rhs; // Assignment evaluates to the assigned value
        }
        return;
    }

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
