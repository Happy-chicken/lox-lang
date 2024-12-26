#ifndef PARSER_HPP_
#define PARSER_HPP_

#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "Expr.hpp"
#include "Stmt.hpp"

using std::initializer_list;
using std::runtime_error;
using std::shared_ptr;
using std::string;
using std::vector;

class Parser {
public:
    explicit Parser(vector<Token> tokens_) : tokens(tokens_) {}
    vector<shared_ptr<Stmt>> parse();

private:
    vector<Token> tokens;
    int current = 0;
    shared_ptr<Expr<Object>> assignment();
    shared_ptr<Expr<Object>> orExpression();
    shared_ptr<Expr<Object>> andExpression();
    shared_ptr<Expr<Object>> expression();
    template<typename Fn>
    shared_ptr<Expr<Object>>
    binary(Fn func, const std::initializer_list<TokenType> &token_args);
    shared_ptr<Expr<Object>> equality();
    shared_ptr<Expr<Object>> comparison();
    shared_ptr<Expr<Object>> term();
    shared_ptr<Expr<Object>> factor();
    shared_ptr<Expr<Object>> unary();
    shared_ptr<Expr<Object>> prefix();
    shared_ptr<Expr<Object>> postfix();
    shared_ptr<Expr<Object>> lambda();
    std::vector<shared_ptr<Expr<Object>>> list();
    shared_ptr<Expr<Object>> subscript();
    shared_ptr<Expr<Object>> finishSubscript(shared_ptr<Expr<Object>> identifier);
    shared_ptr<Expr<Object>> finishCall(shared_ptr<Expr<Object>> callee);
    shared_ptr<Expr<Object>> call();
    shared_ptr<Expr<Object>> primary();

    vector<shared_ptr<Stmt>> block();
    shared_ptr<Stmt> declaration();
    shared_ptr<Stmt> classDeclaration();
    shared_ptr<Stmt> varDeclaration();
    shared_ptr<Function> function(string kind);
    shared_ptr<Stmt> statement();
    shared_ptr<Stmt> forStatement();
    shared_ptr<Stmt> ifStatement();
    shared_ptr<Stmt> whileStatement();
    shared_ptr<Stmt> printStatement();
    shared_ptr<Stmt> returnStatement();
    shared_ptr<Stmt> controlStatement();
    shared_ptr<Stmt> expressionStatement();
    shared_ptr<Stmt> tryStatement();
    shared_ptr<Stmt> throwStatement();

    bool match(const initializer_list<TokenType> &types);
    bool check(TokenType type);
    Token advance();
    bool isAtEnd();
    Token peek();
    Token previous();
    Token consume(TokenType type, string message);
    runtime_error error(Token token, string message);
    void synchronize();

    // class ParseError : public runtime_error
    // {
    // public:
    //    ParseError() : runtime_error("") {}
    // };

    // ParseError error(const Token &token, string message) const;
};

#endif// PARSER_HPP_
