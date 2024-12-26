#ifndef RETURN_ERROR_HPP_
#define RETURN_ERROR_HPP_

#include "RuntimeError.hpp"
#include "Token.hpp"
#include <stdexcept>

using std::runtime_error;

class ReturnError : public runtime_error {
public:
  Object value;
  explicit ReturnError(Object value_) : runtime_error(""), value(value_) {}
  const Object &getReturnValue() const { return value; }
};

class BreakException : public RuntimeError {
public:
  explicit BreakException(const Token &token) noexcept
      : RuntimeError(token, "Cannot break outside of a loop."){};
};

class ContinueException : public RuntimeError {
public:
  explicit ContinueException(const Token &token) noexcept
      : RuntimeError(token, "Cannot continue outside of a loop."){};
};
#endif //  RETURN_ERROR_HPP_
