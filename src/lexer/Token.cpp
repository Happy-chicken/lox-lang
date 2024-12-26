
#include <string>

#include "../../include/LoxCallable.hpp"
#include "../../include/LoxClass.hpp"
#include "../../include/LoxInstance.hpp"
#include "../../include/Token.hpp"

using std::string;
using std::to_string;

Token::Token()
    : type(TokenType::NIL), lexeme(""), literal(Object::make_nil_obj()),
      line(-1) {}
Token::Token(TokenType type, string lexeme, Object literal, int line)
    : type(type), lexeme(lexeme), literal(literal), line(line) {}

string Token::toString() {
  return to_string(type) + " " + lexeme + " " + literal.toString();
}
