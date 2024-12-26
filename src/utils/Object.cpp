#include "../../include/Object.hpp"
#include <memory>
#include <string>
using std::shared_ptr;
using std::string;
using std::to_string;

string Object::toString() {
  switch (data.index()) {
  case 0:
    return std::get<string>(data);
  case 2:
    return std::get<bool>(data) ? "true" : "false";
  case 3:
    return "nil";
  case 4:
    return "<constant list>";
  case 5:
    return "<obj func>";
  case 6:
    return "<obj instance>";
    // return instance->toString();
  case 7:
    return "<obj class>";
    // return lox_class->toString();
  default:
    return to_string(std::get<double>(data));
  }
}

Object Object::make_nil_obj() {
  Object nil_obj;
  nil_obj.data = std::monostate{};
  return nil_obj;
}

Object Object::make_class_obj(shared_ptr<LoxClass> lox_class_) {
  Object class_obj;
  class_obj.data = lox_class_;
  return class_obj;
}