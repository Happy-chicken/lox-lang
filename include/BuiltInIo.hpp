#ifndef BUILTIN_IO_HPP
#define BUILTIN_IO_HPP

#include "LoxCallable.hpp"
#include "LoxFunction.hpp"
#include <iostream>
#include <sstream>
class PrintCallable : public LoxCallable {
public:
  explicit PrintCallable(size_t arity = 0) : aritys{arity} {}

  size_t arity() override;

  Object call(shared_ptr<Interpreter> interpreter,
              vector<Object> args) override;

  std::string toString() override;

private:
  size_t aritys;
};

class InputCallable : public LoxCallable {
public:
  explicit InputCallable(size_t arity = 0) : aritys{arity} {}

  size_t arity() override;

  Object call(shared_ptr<Interpreter> interpreter,
              vector<Object> args) override;

  std::string toString() override;

private:
  size_t aritys;
};

std::string stringify(const Object &item, std::stringstream &stream);
#endif // BUILTIN_IO_HPP
