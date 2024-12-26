#ifndef SCANNER_HPP_
#define SCANNER_HPP_

#include "Token.hpp"
#include <map>
#include <string>
#include <unordered_map>
#include <vector>
using std::map;
using std::string;
using std::unordered_map;
using std::vector;

class Scanner {
private:
  string source;
  vector<Token> tokens;
  static const unordered_map<string, TokenType> keywords;
  size_t start = 0;
  size_t current = 0;
  int line = 1;
  void scanToken();
  char advance();
  void addToken(TokenType type);
  void addToken(TokenType type, Object literal);
  bool match(char expected);

  void String();
  void Number();
  void identifier();

  char peek() const;
  char peekNext() const;
  inline bool isAlpha(char c) const;
  inline bool isAlphaNumeric(char c) const;
  inline bool isDigit(char c) const;
  inline bool isAtEnd() const;

public:
  explicit Scanner(string source);
  vector<Token> scanTokens();
};

#endif // SCANNER_HPP_
