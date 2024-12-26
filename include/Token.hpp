#ifndef TOKEN_HPP_
#define TOKEN_HPP_
#include "Object.hpp"
#include <string>
using std::string;
typedef enum {
  LEFT_PAREN,
  RIGHT_PAREN,
  LEFT_BRACE,
  RIGHT_BRACE,
  COMMA,
  DOT,
  MINUS,
  PLUS,
  CARAT,
  SEMICOLON,
  SLASH,
  STAR,
  BACKSLASH,
  MODULO,
  COLON,

  // One or two character tokens.
  RIGHT_BRACKET, // [
  LEFT_BRACKET,  // ]
  BANG,
  BANG_EQUAL,
  EQUAL,
  EQUAL_EQUAL,
  GREATER,
  GREATER_EQUAL,
  LESS,
  LESS_EQUAL,
  MINUS_MINUS,
  PLUS_PLUS,

  // Literals.
  IDENTIFIER,
  STRING,
  NUMBER,

  // Keywords.
  AND,
  CLASS,
  ELSE,
  ELIF,
  FALSE,
  FUN,
  FOR,
  IF,
  NIL,
  OR,
  PRINT,
  RETURN,
  SUPER,
  THIS,
  TRUE,
  VAR,
  WHILE,
  BREAK,
  CONTINUE,
  LAMBDA,
  TRY,
  THROW,

  TOKEN_EOF
} TokenType;

class Token {
public:
  Token();
  Token(TokenType type, string lexeme, Object literal, int line);
  string toString();
  TokenType type;
  string lexeme;
  Object literal;
  int line;
};

#endif // TOKEN_HPP_
