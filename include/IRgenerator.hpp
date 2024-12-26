#include "./Expr.hpp"
#include "./Stmt.hpp"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include <map>
#include <memory>
// using namespace llvm;


class CodeGenerator : public Visitor<Object>,
                      public Visitor_Stmt,
                      public std::enable_shared_from_this<CodeGenerator> {

public:
    Object visitLiteralExpr(shared_ptr<Literal<Object>> expr);
    Object visitAssignExpr(shared_ptr<Assign<Object>> expr);
    Object visitBinaryExpr(shared_ptr<Binary<Object>> expr);
    Object visitGroupingExpr(shared_ptr<Grouping<Object>> expr);
    Object visitUnaryExpr(shared_ptr<Unary<Object>> expr);
    Object visitVariableExpr(shared_ptr<Variable<Object>> expr);
    Object visitLogicalExpr(shared_ptr<Logical<Object>> expr);

    Object visitIncrementExpr(shared_ptr<Increment<Object>> expr);
    Object visitDecrementExpr(shared_ptr<Decrement<Object>> expr);
    Object visitListExpr(shared_ptr<List<Object>> expr);
    Object visitSubscriptExpr(shared_ptr<Subscript<Object>> expr);
    // TODO
    // Object visitLambdaExpr(shared_ptr<Lambda<R>> expr);

    Object visitCallExpr(shared_ptr<Call<Object>> expr);
    Object visitGetExpr(shared_ptr<Get<Object>> expr);
    Object visitSetExpr(shared_ptr<Set<Object>> expr);
    Object visitThisExpr(shared_ptr<This<Object>> expr);
    Object visitSuperExpr(shared_ptr<Super<Object>> expr);

    void visitExpressionStmt(const Expression &stmt);
    void visitPrintStmt(const Print &stmt);
    void visitVarStmt(const Var &stmt);
    void visitBlockStmt(const Block &stmt);
    void visitClassStmt(const Class &stmt);
    void visitIfStmt(const If &stmt);
    void visitWhileStmt(const While &stmt);
    void visitFunctionStmt(shared_ptr<Function> stmt);
    void visitReturnStmt(const Return &stmt);
    void visitBreakStmt(const Break &stmt);
    void visitContinueStmt(const Continue &stmt);

    void codegenerate(vector<shared_ptr<Stmt>> &statements);
    void codegenerate(shared_ptr<Stmt> stmt);
    void codegenerate(shared_ptr<Expr<Object>> expr);
};
