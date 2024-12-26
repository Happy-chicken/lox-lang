#ifndef LOXCLASS_HPP_
#define LOXCLASS_HPP_

#include "Interpreter.hpp"
#include "LoxCallable.hpp"
#include "LoxFunction.hpp"
#include "Token.hpp"
#include <map>
#include <memory>
#include <string>
#include <vector>

using std::map;
using std::shared_ptr;
using std::string;
using std::vector;

class LoxClass : public LoxCallable {
public:
  string name;
  shared_ptr<LoxClass> superclass;
  map<string, shared_ptr<LoxFunction>> methods;

  shared_ptr<LoxFunction> findMethod(string name);
  explicit LoxClass(string name_, shared_ptr<LoxClass> superclass_,
                    map<string, shared_ptr<LoxFunction>> methods_);

  Object call(shared_ptr<Interpreter> interpreter, vector<Object> arguments);
  size_t arity();
  string toString();
};

#endif // LOXCLASS_HPP_
