#ifndef LOXFUNCTION_HPP_
#define LOXFUNCTION_HPP_

#include "Environment.hpp"
#include "Interpreter.hpp"
#include "LoxCallable.hpp"
#include "Stmt.hpp"
#include "Token.hpp"
#include <memory>
#include <string>
#include <vector>
using std::shared_ptr;
using std::string;
using std::vector;

class LoxFunction : public LoxCallable {
public:
  shared_ptr<Function> declaration;
  shared_ptr<Environment> closure;
  bool isInitializer;
  explicit LoxFunction(shared_ptr<Function> declaration_,
                       shared_ptr<Environment> closure_, bool isInitializer_);

  size_t arity();

  Object call(shared_ptr<Interpreter> interpreter, vector<Object> arguments);

  string toString();
  shared_ptr<LoxFunction> bind(shared_ptr<LoxInstance> instance);
};

#endif // LOXFUNCTION_HPP_
