
#include <memory>
#include <string>
#include <variant>

#include "../../include/BuiltInClass.hpp"
#include "../../include/LoxClass.hpp"
#include "../../include/LoxInstance.hpp"
#include "../../include/RuntimeError.hpp"
#include "../../include/Token.hpp"
using std::string;

LoxInstance::LoxInstance(LoxClass klass_) : klass(klass_) {}

string LoxInstance::toString() { return klass.name + " instance"; }

Object LoxInstance::get(Token name) {
  auto searched = fields.find(name.lexeme);
  if (searched != fields.end()) {
    return searched->second;
  }

  shared_ptr<LoxFunction> method = klass.findMethod(name.lexeme);

  if (method != nullptr) {
    // * here you can add class methods about customed builtin class
    if (auto list_len_method =
            std::dynamic_pointer_cast<ListLenMethods>(method)) {
      return Object::make_obj(list_len_method->bind(
          std::static_pointer_cast<ListInstance>(shared_from_this())));
    } else if (auto list_append_method =
                   std::dynamic_pointer_cast<ListAppendMethods>(method)) {
      return Object::make_obj(list_append_method->bind(
          std::static_pointer_cast<ListInstance>(shared_from_this())));
    }
    return Object::make_obj(method->bind(shared_from_this()));
  }

  throw RuntimeError(name, "Undefined property '" + name.lexeme + "'.");
}

void LoxInstance::set(Token name, Object value) { fields[name.lexeme] = value; }