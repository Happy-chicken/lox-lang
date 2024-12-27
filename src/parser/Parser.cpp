#include <initializer_list>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "../../include/Expr.hpp"
#include "../../include/Logger.hpp"
#include "../../include/Parser.hpp"
#include "../../include/Stmt.hpp"
#include "Token.hpp"

using std::initializer_list;
using std::runtime_error;
using std::shared_ptr;
using std::to_string;
using std::vector;

vector<shared_ptr<Stmt>> Parser::parse() {
    vector<shared_ptr<Stmt>> statements;
    while (!isAtEnd()) {
        statements.emplace_back(declaration());
    }
    return statements;
}

// main part of a parser ==> Stmt...

/// @brief parser a block
/// @return list of statements
vector<shared_ptr<Stmt>> Parser::block() {
    vector<shared_ptr<Stmt>> statements;

    while (!check(RIGHT_BRACE) && !isAtEnd()) {
        statements.emplace_back(declaration());
    }
    consume(RIGHT_BRACE, "Syntax Error. Expect '}' after block.");
    return statements;
}

shared_ptr<Stmt> Parser::declaration() {
    try {
        if (match({CLASS}))
            return classDeclaration();
        if (match({FUN}))
            return function("function");
        if (match({VAR}))
            return varDeclaration();
        return statement();
    } catch (runtime_error const &error) {
        synchronize();
        return nullptr;
    }
}
/// @brief parsre a variable declaration, like var a;
/// @return var Stmt Node, one parsed data
shared_ptr<Stmt> Parser::varDeclaration() {
    Token identifier = consume(IDENTIFIER, "Syntax Error. Expect variable name.");
    shared_ptr<Expr<Object>> initializer =
        match({EQUAL}) ? expression() : nullptr;
    consume(SEMICOLON, "Syntax Error. Expect ';' after variable declaration.");
    shared_ptr<Stmt> var = std::make_shared<Var>(identifier, initializer);
    return var;
}

/// @brief parse the function declaration
/// @param kind the kind of function
/// @return
shared_ptr<Function> Parser::function(string kind) {
    Token identifier =
        consume(IDENTIFIER, "Syntax Error. Expect " + kind + " name.");
    consume(LEFT_PAREN, "Syntax Error. Expect '(' after " + kind + " name.");
    vector<Token> parameters;
    if (!check(RIGHT_PAREN)) {
        do {
            if (parameters.size() >= 255) {
                error(peek(), "Syntax Error. Cannot have more than 255 parameters.");
            }
            parameters.emplace_back(consume(IDENTIFIER, "Expect parameter name."));
        } while (match({COMMA}));
    }
    consume(RIGHT_PAREN, "Syntax Error. Expect ')' after parameters.");
    consume(LEFT_BRACE, "Syntax Error. Expect '{' before " + kind + " body.");
    auto body = block();
    auto func = std::make_shared<Function>(identifier, parameters, body);
    return func;
}

/// @brief parse the class declaration
/// @return
shared_ptr<Stmt> Parser::classDeclaration() {
    Token identifier = consume(IDENTIFIER, "Syntax Error. Expect class name.");

    shared_ptr<Variable<Object>> superclass;
    if (match({LESS})) {
        consume(IDENTIFIER, "Syntax Error. Expect superclass name.");
        superclass = std::make_shared<Variable<Object>>(previous());
    }
    consume(LEFT_BRACE, "Syntax Error. Expect '{' before class body.");

    vector<shared_ptr<Function>> methods;
    while (!check(RIGHT_BRACE) && !isAtEnd()) {
        methods.push_back(function("method"));
    }
    consume(RIGHT_BRACE, "Syntax Error. Expect '}' after class body.");

    auto class_decl = std::make_shared<Class>(identifier, superclass, methods);
    return class_decl;
}

shared_ptr<Stmt> Parser::statement() {
    if (match({FOR}))
        return forStatement();
    if (match({IF}))
        return ifStatement();
    if (match({PRINT}))
        return printStatement();
    if (match({RETURN}))
        return returnStatement();
    if (match({WHILE}))
        return whileStatement();
    if (match({BREAK, CONTINUE}))
        return controlStatement();
    if (match({TRY}))
        return tryStatement();
    if (match({THROW}))
        return throwStatement();
    if (match({LEFT_BRACE}))
        return std::make_shared<Block>(block());
    return expressionStatement();
}

/// @brief parse an expression statement
/// @return
shared_ptr<Stmt> Parser::expressionStatement() {
    shared_ptr<Expr<Object>> expr = expression();
    consume(SEMICOLON, "Syntax Error. Expect ';' after expression.");
    return std::make_shared<Expression>(expr);
}

/// @brief parse a for statement, like for(...)
/// @return
shared_ptr<Stmt> Parser::forStatement() {
    consume(LEFT_PAREN, "Syntax Error. Expect '(' after 'for'.");
    shared_ptr<Stmt> initializer;
    if (match({SEMICOLON}))
        initializer = nullptr;
    else if (match({VAR}))
        initializer = varDeclaration();
    else
        initializer = expressionStatement();

    shared_ptr<Expr<Object>> condition =
        !check(SEMICOLON) ? assignment() : nullptr;
    consume(SEMICOLON, "Syntax Error. Expect ';' after loop condition.");

    shared_ptr<Expr<Object>> increment =
        !check(RIGHT_PAREN) ? assignment() : nullptr;
    consume(RIGHT_PAREN, "Syntax Error. Expect ')' after for clauses.");

    shared_ptr<Stmt> body = statement();

    if (increment != nullptr) {
        vector<shared_ptr<Stmt>> stmts;
        stmts.emplace_back(body);
        stmts.emplace_back(std::make_shared<Expression>(increment));
        // stmts.emplace_back(shared_ptr<Stmt>(new Expression(increment)));
        body = std::make_shared<Block>(stmts);
    }

    if (condition == nullptr) {
        condition = std::make_shared<Literal<Object>>(Object::make_obj(true));
    }
    body = std::make_shared<While>(condition, body);

    if (initializer != nullptr) {
        vector<shared_ptr<Stmt>> stmts2;
        stmts2.emplace_back(initializer);
        stmts2.emplace_back(body);
        body = std::make_shared<Block>(stmts2);
    }

    return body;
}

/// @brief parse a if statement, like if(...)
/// @return
shared_ptr<Stmt> Parser::ifStatement() {
    consume(LEFT_PAREN, "Syntax Error. Expect '(' after 'if'.");
    shared_ptr<Expr<Object>> if_condition = expression();
    consume(RIGHT_PAREN, "Syntax Error. Expect ')' after if condition.");
    shared_ptr<Stmt> then_branch = statement();
    IfBranch main_brach{std::move(if_condition), std::move(then_branch)};
    vector<IfBranch> elif_branches;
    while (match({ELIF})) {
        consume(LEFT_PAREN, "Syntax Error. Expect '(' after 'elif'.");
        auto elif_condition = assignment();
        consume(RIGHT_PAREN, "Syntax Error. Expect ')' after elif condition.");
        auto elif_branch = statement();
        elif_branches.emplace_back(std::move(elif_condition), std::move(elif_branch));
    }
    auto else_branch = match({ELSE}) ? statement() : nullptr;

    return std::make_shared<If>(main_brach, elif_branches, else_branch);
    // return shared_ptr<Stmt>(new If(main_brach, elif_branches, else_branch));
}

/// @brief parse a print statement, like print(...)
/// @return
// TODO
shared_ptr<Stmt> Parser::printStatement() {
    // ! deperate usage, this didn't view print as a function
    // shared_ptr<Expr<Object>> value = expression();
    // consume(SEMICOLON, "Expect ';' after value.");
    // shared_ptr<Stmt> print(new Print(value));
    // return print;
    Token identifier = previous();
    if (!match({LEFT_PAREN})) {
        throw error(identifier, "Syntax Error. Expect '(' after 'print'.");
    }
    auto expr = finishCall(std::make_shared<Variable<Object>>(identifier));
    consume(SEMICOLON, "Syntax Error. Expect ';' after value.");
    return std::make_shared<Print>(expr);
}

/// @brief parse a return statement
/// @return
shared_ptr<Stmt> Parser::returnStatement() {
    Token keyword = previous();
    auto value = check(TokenType::SEMICOLON) ? nullptr : expression();
    consume(SEMICOLON, "Syntax Error. Expect ';' after return value.");
    return std::make_shared<Return>(keyword, value);
}

/// @brief parse a while statement, like while(...)
/// @return
shared_ptr<Stmt> Parser::whileStatement() {
    consume(LEFT_PAREN, "Syntax Error. Expect '(' after 'while'.");
    shared_ptr<Expr<Object>> condition = expression();
    consume(RIGHT_PAREN, "Syntax Error. Expect ')' after condition.");
    shared_ptr<Stmt> body = statement();
    return std::make_shared<While>(condition, body);
}

/// @brief parse a control statement, like {breal; continue}
/// @return
shared_ptr<Stmt> Parser::controlStatement() {
    Token keyword = previous();
    if (keyword.type == BREAK) {
        consume(SEMICOLON, "Syntax Error. Syntax Error. Expect ';' after 'break'.");
        return std::make_shared<Break>(keyword);
    } else if (keyword.type == CONTINUE) {
        consume(SEMICOLON, "Syntax Error. Syntax Error. Expect ';' after 'continue'.");
        return std::make_shared<Continue>(keyword);
    }
    return nullptr;
}

/// @brief parse a try statement, like try{...}
/// @return
shared_ptr<Stmt> Parser::tryStatement() {
    // TODO
    return nullptr;
}

/// @brief parse a throw statement, like throw ...
/// @return
shared_ptr<Stmt> Parser::throwStatement() {
    // TODO
    return nullptr;
}

// ==> Expr...

/// @brief parse the expressions
/// @return
shared_ptr<Expr<Object>> Parser::expression() { return assignment(); }

/// @brief parse the assignment, like a=1
/// @return
shared_ptr<Expr<Object>> Parser::assignment() {
    shared_ptr<Expr<Object>> expr = orExpression();
    if (match({EQUAL})) {
        Token equals = previous();
        shared_ptr<Expr<Object>> value = assignment();

        if (auto subscript = dynamic_cast<Subscript<Object> *>(expr.get());
            subscript != nullptr) {
            Token name = subscript->identifier;
            return std::make_shared<Subscript<Object>>(
                std::move(name), subscript->index, std::move(value)
            );
        }
        if (auto variable = dynamic_cast<Variable<Object> *>(expr.get());
            variable != nullptr) {
            Token name = variable->name;
            return std::make_shared<Assign<Object>>(name, value);
        }
        if (auto get = dynamic_cast<Get<Object> *>(expr.get()); get != nullptr) {
            return std::make_shared<Set<Object>>(get->object, get->name, value);
        }
        error(equals, "Syntax Error. Invalid assignment target.");
    }
    return expr;
}

/// @brief parse the or expression, like 1 or 2
/// @return
shared_ptr<Expr<Object>> Parser::orExpression() {
    shared_ptr<Expr<Object>> expr = andExpression();
    while (match({OR})) {
        Token operation = previous();
        shared_ptr<Expr<Object>> right = andExpression();
        expr = std::make_shared<Logical<Object>>(expr, operation, right);
    }
    return expr;
}

/// @brief parse the and expression, like 1 and 2
/// @return
shared_ptr<Expr<Object>> Parser::andExpression() {
    shared_ptr<Expr<Object>> expr = equality();
    while (match({AND})) {
        Token operation = previous();
        shared_ptr<Expr<Object>> right = equality();
        expr = std::make_shared<Logical<Object>>(expr, operation, right);
    }
    return expr;
}

template<typename Fn>
shared_ptr<Expr<Object>>
Parser::binary(Fn func, const std::initializer_list<TokenType> &token_args) {
    auto expr = func();
    while (match(token_args)) {
        auto op = previous();
        auto right = func();
        expr = std::make_shared<Binary<Object>>(expr, op, right);
    }
    return expr;
}

/// @brief parse an equality expreeion, like a!=b, a==b
/// @return
shared_ptr<Expr<Object>> Parser::equality() {
    auto expr =
        binary([this]() { return comparison(); }, {BANG_EQUAL, EQUAL_EQUAL});
    return expr;
}

/// @brief parse a comparsion expreesion, like a<b, a<=b, a>b, a>=b
/// @return
shared_ptr<Expr<Object>> Parser::comparison() {
    auto expr = binary([this]() { return term(); }, {TokenType::GREATER, TokenType::GREATER_EQUAL, TokenType::LESS, TokenType::LESS_EQUAL});
    return expr;
}

/// @brief parse a term expression, like a+b, a-b
/// @return
shared_ptr<Expr<Object>> Parser::term() {
    auto expr = binary([this]() { return factor(); }, {TokenType::MINUS, TokenType::PLUS});
    return expr;
}

/// @brief parse factor expreesion, like a^b, a/b, a*b, a%b
/// @return
shared_ptr<Expr<Object>> Parser::factor() {
    auto expr = binary([this]() { return unary(); }, {TokenType::SLASH, TokenType::BACKSLASH, TokenType::STAR, TokenType::MODULO});
    return expr;
}

shared_ptr<Expr<Object>> Parser::unary() {
    if (match({BANG, MINUS})) {
        Token operation = previous();
        shared_ptr<Expr<Object>> right = unary();
        return std::make_shared<Unary<Object>>(operation, right);
    }
    return prefix();
}

/// @brief parse a prefix expreesion, like ++a, --a
/// @return
shared_ptr<Expr<Object>> Parser::prefix() {
    if (match({PLUS_PLUS, MINUS_MINUS})) {
        const auto op = previous();
        auto lvalue = consume(IDENTIFIER, "Syntax Error. Operators '++' and '--' "
                                          "must be applied to and lvalue operand.");

        if (lvalue.type == PLUS_PLUS || lvalue.type == MINUS_MINUS) {
            throw error(
                peek(),
                "Syntax Error. Operators '++' and '--' cannot be concatenated."
            );
        }

        if (op.type == PLUS_PLUS) {
            return std::make_shared<Increment<Object>>(
                lvalue, Increment<Object>::Type::PREFIX
            );
        } else {
            return std::make_shared<Decrement<Object>>(
                lvalue, Decrement<Object>::Type::PREFIX
            );
        }
    }

    return postfix();
}
/// @brief parse a postfix expression, like a++, a--. etc.
/// @return
shared_ptr<Expr<Object>> Parser::postfix() {
    auto expr = call();

    if (match({TokenType::PLUS_PLUS, TokenType::MINUS_MINUS})) {
        const auto op = previous();
        // Forbid incrementing/decrementing rvalues.
        if (!dynamic_cast<Variable<Object> *>(expr.get())) {
            throw error(op, "Syntax Error. Operators '++' and '--' must be applied "
                            "to and lvalue operand.");
        }

        // Concatenating increment/decrement operators is not allowed.
        if (match({TokenType::PLUS_PLUS, TokenType::MINUS_MINUS})) {
            throw error(
                op, "Syntax Error. Operators '++' and '--' cannot be concatenated."
            );
        }

        if (op.type == TokenType::PLUS_PLUS) {
            expr = std::make_shared<Increment<Object>>(
                dynamic_cast<Variable<Object> *>(expr.get())->name,
                Increment<Object>::Type::POSTFIX
            );
        } else {
            expr = std::make_shared<Decrement<Object>>(
                dynamic_cast<Variable<Object> *>(expr.get())->name,
                Decrement<Object>::Type::POSTFIX
            );
        }
    }

    return expr;
}

/// @brief parse a call expression, like fub()
/// @return
shared_ptr<Expr<Object>> Parser::call() {
    shared_ptr<Expr<Object>> expr = subscript();
    while (true) {
        if (match({LEFT_PAREN})) {
            expr = finishCall(expr);
        } else if (match({DOT})) {
            Token name =
                consume(IDENTIFIER, "Syntax Error. Expect property name after '.'.");
            expr = std::make_shared<Get<Object>>(expr, name);
        } else {
            break;
        }
    }
    return expr;
}

/// @brief finish parsing a call exapression
/// @param expr callee
/// @return
shared_ptr<Expr<Object>> Parser::finishCall(shared_ptr<Expr<Object>> callee) {
    vector<shared_ptr<Expr<Object>>> arguments;
    if (!check(RIGHT_PAREN)) {
        do {
            if (arguments.size() >= 255) {
                error(peek(), "Syntax Error. Cannot have more than 255 arguments.");
            }
            arguments.push_back(expression());
        } while (match({COMMA}));
    }

    Token paren =
        consume(RIGHT_PAREN, "Syntax Error. Expect ')' after arguments.");

    return std::make_shared<Call<Object>>(callee, paren, arguments);
}

/// @brief parse a subscript expression, like a[]
/// @return
shared_ptr<Expr<Object>> Parser::subscript() {
    shared_ptr<Expr<Object>> expr = primary();

    while (true) {
        if (match({TokenType::LEFT_BRACKET})) {
            expr = finishSubscript(std::move(expr));
        } else {
            break;
        }
    }

    return expr;
}

/// @brief finih parsing, that is implement
/// @param expr
/// @return
shared_ptr<Expr<Object>>
Parser::finishSubscript(shared_ptr<Expr<Object>> identifier) {
    auto index = orExpression();
    consume(TokenType::RIGHT_BRACKET, "Syntax Error. Expect ']' after arguments.");

    // Forbid calling rvalues.
    if (!dynamic_cast<Variable<Object> *>(identifier.get())) {
        throw error(peek(), "Syntax Error. Object is not subscriptable.");
    }

    auto var = dynamic_cast<Variable<Object> *>(identifier.get())->name;

    return std::make_shared<Subscript<Object>>(var, index, nullptr);
}

/// @brief parse the lambda function
/// @return
shared_ptr<Expr<Object>> Parser::lambda() {
    return nullptr;
    // TODO
    // vector<Token> parameters;
    // while (true)
    // {
    // 	if (parameters.size() >= 255)
    // 		error(previous(), "Syntax Error. Can't have more than 255
    // parameters."); 	if (!match({IDENTIFIER})) 		break;
    // 	parameters.emplace_back(previous());
    // 	if (!match({COMMA}))
    // 		break;
    // }
    // void_cast(consume(COLON, "Syntax Error. Expect ':' after lambda
    // parameters.")); shared_ptr<Stmt> body{std::make_shared<Return>(previous(),
    // assignment())}; return shared_ptr<Expr<Object>>(new
    // Lambda<Object>(std::move(parameters), std::move(body)));
    // std::make_shared<Lambda>(std::move(parameters), std::move(body));
}

/// @brief parse a primary expression
/// @return
shared_ptr<Expr<Object>> Parser::primary() {
    if (match({FALSE})) {
        return std::make_shared<Literal<Object>>(Object::make_obj(false));
    }
    if (match({TRUE})) {
        return std::make_shared<Literal<Object>>(Object::make_obj(true));
    }
    if (match({NIL})) {
        return std::make_shared<Literal<Object>>(Object::make_nil_obj());
    }

    if (match({NUMBER})) {
        return std::make_shared<Literal<Object>>(
            Object::make_obj(std::get<double>(previous().literal.data))
        );
    }
    if (match({INTEGEL})) {
        return std::make_shared<Literal<Object>>(
            Object::make_obj(std::get<int>(previous().literal.data))
        );
    }
    if (match({STRING})) {
        return std::make_shared<Literal<Object>>(
            Object::make_obj(std::get<std::string>(previous().literal.data))
        );
    }

    if (match({SUPER})) {
        Token keyword = previous();
        consume(DOT, "Syntax Error. Expect '.' after 'super'.");
        Token method =
            consume(IDENTIFIER, "Syntax Error. Expect superclass method name.");
        return std::make_shared<Super<Object>>(keyword, method);
    }

    if (match({THIS})) {
        return std::make_shared<This<Object>>(previous());
    }

    if (match({IDENTIFIER})) {
        return std::make_shared<Variable<Object>>(previous());
    }

    if (match({LEFT_PAREN})) {
        shared_ptr<Expr<Object>> expr = expression();
        consume(RIGHT_PAREN, "Syntax Error. Expect ')' after expression.");
        return std::make_shared<Grouping<Object>>(expr);
    }

    if (match({LEFT_BRACKET})) {
        Token opening_bracket = previous();
        auto expr = list();
        consume(RIGHT_BRACKET, "Syntax Error. Expect ']' at the end of a list");
        return std::make_shared<List<Object>>(opening_bracket, expr);
    }
    throw error(peek(), "Syntax Error. Expect expression.");
}

/// @brief parse a list expression, a = []
/// @return
vector<shared_ptr<Expr<Object>>> Parser::list() {
    vector<shared_ptr<Expr<Object>>> items;
    // Return an empty list if there are no values.
    if (check(RIGHT_BRACKET))
        return items;
    do {
        if (check(RIGHT_BRACKET))
            break;

        items.emplace_back(orExpression());

        if (items.size() > 100)
            error(peek(), "Syntax Error. Cannot have more than 100 items in a list.");

    } while (match({COMMA}));

    return items;
}

// helper functions...

/// @brief get the previous token
/// @return  previous token
Token Parser::previous() { return tokens[current - 1]; }

// ? return the reference of current token?
/// @brief get the current token
/// @return  current token
Token Parser::peek() { return tokens[current]; }

/// @brief check if the parser is at the end of tokens
/// @return
bool Parser::isAtEnd() { return peek().type == TOKEN_EOF; }

/// @brief advance the parser
/// @return current token
Token Parser::advance() {
    if (!isAtEnd()) {
        current++;
    }
    return previous();
}

/// @brief check if the current token is of a certain type
/// @param type expected toekn type
/// @return
bool Parser::check(TokenType type) {
    if (isAtEnd()) {
        return false;
    }
    return peek().type == type;
}

/// @brief Check if the current token is of any of the given types
/// @param types types list to be checked, one or more
/// @return
bool Parser::match(const initializer_list<TokenType> &types) {
    for (auto type: types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

/// @brief consume a token and if an error occur, raise parse error
/// @param type expected token type
/// @param message error msg
/// @return
Token Parser::consume(TokenType type, string message) {
    if (check(type))
        return advance();
    throw error(peek(), message);
}

// error handle function and recovery
runtime_error Parser::error(Token token, string message) {
    Error::addError(token, std::move(message));
    if (token.type == TOKEN_EOF) {
        return runtime_error(to_string(token.line) + " at end" + message);
    } else {
        return runtime_error(to_string(token.line) + " at '" + token.lexeme + "'" + message);
    }
}

void Parser::synchronize() {
    advance();
    while (!isAtEnd()) {
        if (previous().type == SEMICOLON)
            return;
        switch (peek().type) {
            case CLASS:
            case FUN:
            case VAR:
            case FOR:
            case IF:
            case WHILE:
            case PRINT:
            case RETURN:
            case BREAK:
                return;
            default:
                advance();
        }
    }
}
