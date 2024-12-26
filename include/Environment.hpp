#ifndef ENVIRONMENT_HPP_
#define ENVIRONMENT_HPP_

#include "Token.hpp"
#include <memory>
#include <string>
#include <unordered_map>

using std::shared_ptr;
using std::string;
using std::unordered_map;

class Environment : public std::enable_shared_from_this<Environment> {
public:
  Environment() = default;
  explicit Environment(shared_ptr<Environment> enclosing);
  void define(string name, Object value);
  void assign(Token name, Object value);
  Object get(Token name);
  Object getAt(int distance, string name);
  void assignAt(int distance, Token name, Object value);
  unordered_map<string, Object> getValues() const { return values; }
  shared_ptr<Environment> ancestor(int distance);
  shared_ptr<Environment> enclosing;

private:
  unordered_map<string, Object> values;
};

#endif // ENVIRONMENT_HPP_
