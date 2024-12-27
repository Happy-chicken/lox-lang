#include <iostream>
#include <string>
#include <vector>

#include "../../include/Logger.hpp"
#include "../../include/Scanner.hpp"
#include "../../include/Token.hpp"
#include "../../include/lox.hpp"

using std::stod;
using std::string;
using std::vector;

const unordered_map<string, TokenType> Scanner::keywords = {
    {"and", AND},
    {"class", CLASS},
    {"else", ELSE},
    {"false", FALSE},
    {"for", FOR},
    {"fun", FUN},
    {"if", IF},
    {"nil", NIL},
    {"or", OR},
    {"print", PRINT},
    {"return", RETURN},
    {"super", SUPER},
    {"this", THIS},
    {"true", TRUE},
    {"var", VAR},
    {"while", WHILE},
};

Scanner::Scanner(string source) : source(std::move(source)) {}

vector<Token> Scanner::scanTokens() {
    while (!isAtEnd()) {
        // We are at the beginning of the next lexeme.
        start = current;
        scanToken();
    }

    Token token(TOKEN_EOF, "", Object::make_obj(""), line);
    tokens.push_back(token);
    return tokens;
}

// helper function
inline bool Scanner::isAtEnd() const { return current >= source.size(); }

inline bool Scanner::isDigit(char c) const { return c >= '0' && c <= '9'; }

inline bool Scanner::isAlpha(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

inline bool Scanner::isAlphaNumeric(char c) const {
    return isAlpha(c) || isDigit(c);
}

char Scanner::peek() const { return isAtEnd() ? '\0' : source.at(current); }

char Scanner::peekNext() const {
    return current + 1 >= source.size() ? '\0' : source.at(current + 1);
}

char Scanner::advance() {
    current++;
    return source.at(current - 1);
}

void Scanner::addToken(TokenType type) { addToken(type, Object::make_obj("")); }

void Scanner::addToken(TokenType type, Object literal) {
    string text = source.substr(start, current - start);
    Token token(type, text, literal, line);
    tokens.push_back(token);
}

bool Scanner::match(char expected) {
    if (isAtEnd())
        return false;
    if (source.at(current) != expected)
        return false;
    current++;
    return true;
}

void Scanner::scanToken() {
    char c = advance();

    switch (c) {
        case '(':
            addToken(LEFT_PAREN);
            break;
        case ')':
            addToken(RIGHT_PAREN);
            break;
        case '{':
            addToken(LEFT_BRACE);
            break;
        case '}':
            addToken(RIGHT_BRACE);
            break;
        case '[':
            addToken(LEFT_BRACKET);
            break;
        case ']':
            addToken(RIGHT_BRACKET);
            break;
        case ',':
            addToken(COMMA);
            break;
        case '.':
            addToken(DOT);
            break;
        case '-':
            addToken(match('-') ? MINUS_MINUS : MINUS);
            break;
        case '+':
            addToken(match('+') ? PLUS_PLUS : PLUS);
            break;
        case ';':
            addToken(SEMICOLON);
            break;
        case '*':
            addToken(STAR);
            break;
        case '^':
            addToken(CARAT);
            break;
        case '%':
            addToken(MODULO);
            break;
        case '\\':
            addToken(BACKSLASH);
            break;
        case '!':
            addToken(match('=') ? BANG_EQUAL : BANG);
            break;
        case '=':
            addToken(match('=') ? EQUAL_EQUAL : EQUAL);
            break;
        case '<':
            addToken(match('=') ? LESS_EQUAL : LESS);
            break;
        case '>':
            addToken(match('=') ? GREATER_EQUAL : GREATER);
            break;
        case '/':
            if (match('/')) {
                // A comment goes until the end of the line.
                while (peek() != '\n' && !isAtEnd())
                    advance();
            }// TODO multi-comments/**/
            else if (match('*')) {
                while (!(peek() == '*' && peekNext() == '/') && !isAtEnd()) {
                    if (peek() == '\n') {
                        line++;
                    }
                    advance();
                }
                current += 2;// skip the close token, */
            } else {
                addToken(SLASH);
            }
            break;
        case ' ':
        case '\0':
        case '\r':
        case '\t':
            // Ignore whitespace.
            break;
        case '\n':
            line++;
            break;
        case '"':
            String();
            break;
        default:
            if (isDigit(c)) {
                Number();
            } else if (isAlpha(c)) {
                identifier();
            } else {
                Error::addError(line, "", "Unexpected character '" + std::to_string(c) + "'.");
                // lox::error(line, "Unexpected character.");
            }
            break;
    }
}

void Scanner::String() {
    while (peek() != '"' && !isAtEnd()) {
        if (peek() == '\n')
            line++;
        advance();
    }

    // Unterminated string.
    if (isAtEnd()) {
        Error::addError(line, "", "Unterminated string.");
        // lox::error(line, "Unterminated string.");
        return;
    }

    // The closing ".
    advance();

    // Trim the surrounding quotes.
    string value = source.substr(start + 1, current - start - 2);
    addToken(STRING, Object::make_obj(value));
}

void Scanner::Number() {
    while (isDigit(peek()))
        advance();
    bool isInteger = true;
    // Look for a fractional part.
    if (peek() == '.' && isDigit(peekNext())) {
        // Consume the "."
        isInteger = false;
        advance();
        while (isDigit(peek()))
            advance();
    }

    std::string numStr = source.substr(start, current - start);
    if (isInteger) {
        int num = std::stoi(numStr);
        addToken(INTEGEL, Object::make_obj(num));
    } else {
        double num = std::stod(numStr);
        addToken(NUMBER, Object::make_obj(num));
    }
}

void Scanner::identifier() {
    while (isAlphaNumeric(peek()))
        advance();
    const string text = source.substr(start, current - start);

    addToken(keywords.find(text) != keywords.end() ? keywords.at(text) : IDENTIFIER);
}
