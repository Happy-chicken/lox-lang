#include "../../include/BuiltInFun.hpp"
#include "../../include/LoxClass.hpp"
#include "../../include/LoxInstance.hpp"
#include "../../include/LoxList.hpp"

// Native clock
size_t ClockCallable::arity() { return 0; }

Object ClockCallable::call(shared_ptr<Interpreter> interpreter,
                           std::vector<Object> args) {
  static_assert(std::is_integral_v<std::chrono::system_clock::rep>,
                "Representation of ticks isn't an integral value.");

  // Returns Unix time in seconds.
  auto now = std::chrono::system_clock::now().time_since_epoch();
  return Object::make_obj(static_cast<double>(
      std::chrono::duration_cast<std::chrono::milliseconds>(now).count() /
      1000.0));
}

std::string ClockCallable::toString() { return "<native fn: clock>"; }

// native type
size_t TypeCallable::arity() { return aritys; }

Object TypeCallable::call(shared_ptr<Interpreter> interpreter,
                          std::vector<Object> args) {
  if (args.size() != 1)
    throw std::runtime_error("Type function takes exactly one argument.");
  return Object::make_obj(args[0].toString());
}

std::string TypeCallable::toString() { return "<native fn: type>"; }
