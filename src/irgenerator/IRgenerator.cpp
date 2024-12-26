#include "../../include/IRgenerator.hpp"
#include "Token.hpp"
#include <cstdio>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/raw_ostream.h>


void CodeGenerator::codegenerate(vector<shared_ptr<Stmt>> &statements) {
    for (auto stmt: statements) {
        codegenerate(stmt);
    }
}

void CodeGenerator::codegenerate(shared_ptr<Stmt> stmt) {
    stmt->accept(shared_from_this());
}

void CodeGenerator::codegenerate(shared_ptr<Expr<Object>> expr) {
    expr->accept(shared_from_this());
}

Object CodeGenerator::visitLiteralExpr(shared_ptr<Literal<Object>> expr) {
    return Object::make_nil_obj();
}

Object CodeGenerator::visitAssignExpr(shared_ptr<Assign<Object>> expr) {
    //llvm to generate IR for assignment
    return Object::make_nil_obj();
}

Object CodeGenerator::visitBinaryExpr(shared_ptr<Binary<Object>> expr) {

    return Object::make_nil_obj();
}

Object CodeGenerator::visitGroupingExpr(shared_ptr<Grouping<Object>> expr) {
    return Object::make_nil_obj();
}

Object CodeGenerator::visitUnaryExpr(shared_ptr<Unary<Object>> expr) {
    return Object::make_nil_obj();
}

Object CodeGenerator::visitVariableExpr(shared_ptr<Variable<Object>> expr) {
    return Object::make_nil_obj();
}

Object CodeGenerator::visitLogicalExpr(shared_ptr<Logical<Object>> expr) { return Object::make_nil_obj(); }

Object CodeGenerator::visitIncrementExpr(shared_ptr<Increment<Object>> expr) { return Object::make_nil_obj(); }

Object CodeGenerator::visitDecrementExpr(shared_ptr<Decrement<Object>> expr) { return Object::make_nil_obj(); }

Object CodeGenerator::visitListExpr(shared_ptr<List<Object>> expr) { return Object::make_nil_obj(); }

Object CodeGenerator::visitSubscriptExpr(shared_ptr<Subscript<Object>> expr) { return Object::make_nil_obj(); }

Object CodeGenerator::visitCallExpr(shared_ptr<Call<Object>> expr) { return Object::make_nil_obj(); }

Object CodeGenerator::visitGetExpr(shared_ptr<Get<Object>> expr) { return Object::make_nil_obj(); }

Object CodeGenerator::visitSetExpr(shared_ptr<Set<Object>> expr) { return Object::make_nil_obj(); }

Object CodeGenerator::visitThisExpr(shared_ptr<This<Object>> expr) { return Object::make_nil_obj(); }

Object CodeGenerator::visitSuperExpr(shared_ptr<Super<Object>> expr) { return Object::make_nil_obj(); }

void CodeGenerator::visitExpressionStmt(const Expression &stmt) {
    codegenerate(stmt.expression);
}
void CodeGenerator::visitPrintStmt(const Print &stmt) {}
void CodeGenerator::visitVarStmt(const Var &stmt) {
}
void CodeGenerator::visitBlockStmt(const Block &stmt) {}
void CodeGenerator::visitClassStmt(const Class &stmt) {}
void CodeGenerator::visitIfStmt(const If &stmt) {
}
void CodeGenerator::visitWhileStmt(const While &stmt) {}
void CodeGenerator::visitFunctionStmt(shared_ptr<Function> stmt) {}
void CodeGenerator::visitReturnStmt(const Return &stmt) {}
void CodeGenerator::visitBreakStmt(const Break &stmt) {}
void CodeGenerator::visitContinueStmt(const Continue &stmt) {}