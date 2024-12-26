#ifndef ASTPRINTER_HPP_
#define ASTPRINTER_HPP_

#include "Expr.hpp"
#include <memory>
#include <string>
#include <utility>
#include <vector>

using std::shared_ptr;
using std::string;

class AstPrinter : public Visitor<string>,
                   public std::enable_shared_from_this<AstPrinter> {
public:
  string print(shared_ptr<Expr<string>> expr);
  string visitLiteralExpr(const Literal<string> &expr);
  string visitAssignExpr(const Assign<string> &expr);
  string visitBinaryExpr(const Binary<string> &expr);
  string visitGroupingExpr(const Grouping<string> &expr);
  string visitUnaryExpr(const Unary<string> &expr);
  //  string visitVariableExpr(const Variable<string>& expr) {}
  //  string visitLogicalExpr(const Logical<string>& expr) {}
  //  string visitCallExpr(const Call<string>& expr) {}
  //  string visitGetExpr(const Get& expr);
  //  string visitLogicalExpr(const Logical& expr);
  //  string visitSetExpr(const Set& expr);
  //  string visitSuperExpr(const Super& expr);
  //  string visitThisExpr(const This& expr);
};

#endif // ASTPRINTER_HPP_
