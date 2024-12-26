#ifndef RESOLVER_HPP_
#define RESOLVER_HPP_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "Expr.hpp"
#include "Interpreter.hpp"
#include "Stmt.hpp"
#include "Token.hpp"

using std::map;
using std::shared_ptr;
using std::string;
using std::vector;

class Resolver : public Visitor<Object>,
                 public Visitor_Stmt,
                 public std::enable_shared_from_this<Resolver> {
public:
  vector<map<string, bool>> scopes;
  shared_ptr<Interpreter> interpreter;
  explicit Resolver(shared_ptr<Interpreter> interpreter);
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
  void resolve(vector<shared_ptr<Stmt>> statements);
  void resolve(shared_ptr<Stmt> stmt);
  void resolve(shared_ptr<Expr<Object>> expr);

private:
  enum FunctionType { FUNCTION_NONE, FUNCTION, METHOD, INITIALIZER };
  enum ClassType {
    CLASS_NONE,
    CLASS,
    SUBCLASS,
  };
  ClassType currentClass = CLASS_NONE;
  FunctionType currentFunction = FUNCTION_NONE;
  size_t loop_nesting_level = 0u;
  void beginScope();
  void endScope();
  void declare(Token name);
  void define(Token name);
  void resolveLocal(shared_ptr<Expr<Object>>, Token name);
  void resolveFunction(shared_ptr<Function> function, FunctionType type);
};

#endif // RESOLVER_HPP_
