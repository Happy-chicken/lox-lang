#ifndef LOXCALLABLE_HPP_
#define LOXCALLABLE_HPP_

#include "Interpreter.hpp"
#include "Token.hpp"
#include <memory>
#include <vector>

using std::shared_ptr;
using std::string;
using std::vector;

class LoxCallable {
public:
  virtual ~LoxCallable() = default; // for derived class
  virtual size_t arity() = 0;
  virtual Object call(shared_ptr<Interpreter> interpreter,
                      vector<Object> arguments) = 0;
  virtual string toString() = 0;
};

#endif //  LOXCALLABLE_HPP_
