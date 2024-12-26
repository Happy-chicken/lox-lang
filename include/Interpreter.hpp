#ifndef INTERPRETER_HPP_
#define INTERPRETER_HPP_

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "Environment.hpp"
#include "Expr.hpp"
#include "Stmt.hpp"
#include "Token.hpp"

using std::shared_ptr;
using std::string;
using std::unordered_map;
using std::vector;

class Interpreter : public Visitor<Object>,
                    public Visitor_Stmt,
                    public std::enable_shared_from_this<Interpreter> {
public:
  Interpreter();
  void interpret(vector<shared_ptr<Stmt>> statements);
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
  // Object visitLambdaExpr(shared_ptr<Lambda<Object>> expr);
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

  void executeBlock(vector<shared_ptr<Stmt>> statements,
                    shared_ptr<Environment> environment);
  shared_ptr<Environment> globals = shared_ptr<Environment>(new Environment());
  void resolve(shared_ptr<Expr<Object>> expr, int depth);

  /// @brief initialize with current interpreter and environment, // ? to store
  /// env
  class EnvironmentGuard {
  public:
    EnvironmentGuard(Interpreter &interpreter,
                     std::shared_ptr<Environment> env);

    ~EnvironmentGuard();

  private:
    Interpreter &interpreter;
    std::shared_ptr<Environment> previous_env;
  };

private:
  // ! some fatal error
  shared_ptr<Environment> &environment = globals;
  // Environment *const global_environment;
  unordered_map<shared_ptr<Expr<Object>>, int> locals;
  Object evaluate(shared_ptr<Expr<Object>> expr);
  void execute(shared_ptr<Stmt> stmt);
  bool isTruthy(Object object);
  bool isEqual(Object a, Object b);
  void checkNumberOperand(Token operation, Object operand);
  void checkNumberOperands(Token operation, Object left, Object right);
  string stringify(Object object);
  Object lookUpVariable(Token name, shared_ptr<Expr<Object>> expr);
};

#endif // INTERPRETER_HPP_
