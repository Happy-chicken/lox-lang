#ifndef BUILT_IN_HPP
#define BUILT_IN_HPP

#include "LoxCallable.hpp"
#include "LoxFunction.hpp"
#include <chrono>
#include <iostream>
#include <string>

class ClockCallable : public LoxCallable {
public:
  size_t arity() override;

  Object call(shared_ptr<Interpreter> interpreter,
              vector<Object> args) override;

  std::string toString() override;

private:
  size_t aritys = 0;
};

class TypeCallable : public LoxCallable {
public:
  explicit TypeCallable(size_t arity = 0) : aritys(arity) {}

  size_t arity() override;

  Object call(shared_ptr<Interpreter> interpreter,
              vector<Object> args) override;

  std::string toString() override;

private:
  size_t aritys;
};

#endif // BUILT_IN_HPP