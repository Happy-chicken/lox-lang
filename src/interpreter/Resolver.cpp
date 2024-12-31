// Copyright 2020.2.22 <Copyright hulin>

#include <map>
#include <memory>
#include <vector>

#include "../../include/Expr.hpp"
#include "../../include/Interpreter.hpp"
#include "../../include/Logger.hpp"
#include "../../include/Resolver.hpp"
#include "../../include/Stmt.hpp"
#include "../../include/Token.hpp"
#include "../../include/lox.hpp"

using std::map;
using std::shared_ptr;
using std::vector;

Resolver::Resolver(shared_ptr<Interpreter> interpreter_)
    : interpreter(interpreter_) {}

Object Resolver::visitLiteralExpr(shared_ptr<Literal<Object>> expr) {
    return Object::make_nil_obj();
}

Object Resolver::visitAssignExpr(shared_ptr<Assign<Object>> expr) {
    resolve(expr->value);
    resolveLocal(expr, expr->name);
    return Object::make_nil_obj();
}

Object Resolver::visitBinaryExpr(shared_ptr<Binary<Object>> expr) {
    resolve(expr->left);
    resolve(expr->right);
    return Object::make_nil_obj();
}

Object Resolver::visitGroupingExpr(shared_ptr<Grouping<Object>> expr) {
    resolve(expr->expression);
    return Object::make_nil_obj();
}

Object Resolver::visitUnaryExpr(shared_ptr<Unary<Object>> expr) {
    resolve(expr->right);
    return Object::make_nil_obj();
}

Object Resolver::visitVariableExpr(shared_ptr<Variable<Object>> expr) {
    if (scopes.size() != 0) {
        auto last = scopes.back();
        auto searched = last.find(expr->name.lexeme);
        if (searched != last.end() && !searched->second) {
            Error::addError(expr->name, "Resolvetime Error. Can't read local "
                                        "variable in its own initializer.");
            // lox::error(expr->name.line,
            //            "Resolvetime Error. Cannot read local variable in its own
            //            initializer.");
        }
    }
    resolveLocal(expr, expr->name);
    return Object::make_nil_obj();
}

Object Resolver::visitLogicalExpr(shared_ptr<Logical<Object>> expr) {
    resolve(expr->left);
    resolve(expr->right);
    return Object::make_nil_obj();
}

Object Resolver::visitIncrementExpr(shared_ptr<Increment<Object>> expr) {
    resolveLocal(expr, expr->identifier);
    return Object::make_nil_obj();
}

Object Resolver::visitDecrementExpr(shared_ptr<Decrement<Object>> expr) {
    resolveLocal(expr, expr->identifier);
    return Object::make_nil_obj();
}

Object Resolver::visitListExpr(shared_ptr<List<Object>> expr) {
    for (const auto &e: expr->items) {
        resolve(e);
    }
    return Object::make_nil_obj();
}

Object Resolver::visitSubscriptExpr(shared_ptr<Subscript<Object>> expr) {
    // Resolve the index of the subscript.
    resolve(expr->index);
    // If there's a value associated with the subscript, then it is also resolved.
    if (expr->value) {
        resolve(expr->value);
    }

    // Resolve the variable being accessed.
    resolveLocal(expr, expr->identifier);
    return Object::make_nil_obj();
}

// TODO
// Object Resolver::visitLambdaExpr(shared_ptr<Lambda<Object>> expr)
// {
//     return Object::make_nil_obj();
// }

Object Resolver::visitCallExpr(shared_ptr<Call<Object>> expr) {
    resolve(expr->callee);

    for (auto argument: expr->arguments) {
        resolve(argument);
    }
    return Object::make_nil_obj();
}

Object Resolver::visitGetExpr(shared_ptr<Get<Object>> expr) {
    resolve(expr->object);
    return Object::make_nil_obj();
}

Object Resolver::visitSetExpr(shared_ptr<Set<Object>> expr) {
    resolve(expr->value);
    resolve(expr->object);
    return Object::make_nil_obj();
}

Object Resolver::visitThisExpr(shared_ptr<This<Object>> expr) {
    if (currentClass == CLASS_NONE) {
        Error::addError(expr->keyword, "Resolvetime Error. Cannot use 'this' outside of a class.");
        // lox::error(expr->keyword.line,
        //            "Resolvetime Error. Cannot use 'this' outside of a class.");
        return Object::make_nil_obj();
    }
    resolveLocal(expr, expr->keyword);
    return Object::make_nil_obj();
}

Object Resolver::visitSuperExpr(shared_ptr<Super<Object>> expr) {
    if (currentClass == CLASS_NONE) {
        Error::addError(
            expr->keyword,
            "Resolvetime Error. Cannot use 'super' outside of a class."
        );
        // lox::error(expr->keyword.line,
        //            "Resolvetime Error. Cannot use 'super' outside of a class.");
    } else if (currentClass != SUBCLASS) {
        Error::addError(
            expr->keyword,
            "Resolvetime Error. Cannot use 'super' in a class with no superclass."
        );
        // lox::error(expr->keyword.line,
        //            "Resolvetime Error. Cannot use 'super' in a class with no
        //            superclass.");
    }
    resolveLocal(expr, expr->keyword);
    return Object::make_nil_obj();
}

void Resolver::visitExpressionStmt(const Expression &stmt) {
    resolve(stmt.expression);
}

void Resolver::visitPrintStmt(const Print &stmt) { resolve(stmt.expression); }

void Resolver::visitVarStmt(const Var &stmt) {
    declare(stmt.name);
    if (stmt.initializer != nullptr) {
        resolve(stmt.initializer);
    }
    define(stmt.name);
}

void Resolver::visitBlockStmt(const Block &stmt) {
    beginScope();
    resolve(stmt.statements);
    endScope();
}

void Resolver::visitClassStmt(const Class &stmt) {
    ClassType enclosingClass = currentClass;
    currentClass = CLASS;

    declare(stmt.name);
    define(stmt.name);

    if (stmt.superclass != nullptr &&
        stmt.name.lexeme == stmt.superclass->name.lexeme) {
        Error::addError(stmt.superclass->name, "Resolvetime Error. A class cannot inherit from itself.");
        // lox::error(stmt.superclass->name.line,
        //            "Resolvetime Error. A class cannot inherit from itself.");
    }

    if (stmt.superclass != nullptr) {
        currentClass = SUBCLASS;
        resolve(stmt.superclass);
    }

    if (stmt.superclass != nullptr) {
        beginScope();
        auto backed = scopes.back();
        backed["super"] = true;
        scopes.pop_back();
        scopes.emplace_back(backed);
    }

    beginScope();
    auto back = scopes.back();
    back["this"] = true;
    scopes.pop_back();
    scopes.emplace_back(back);

    for (auto method: stmt.methods) {
        FunctionType declaration = METHOD;
        if (method->functionName.lexeme == "init") {
            declaration = INITIALIZER;
        }
        resolveFunction(method, declaration);
    }

    endScope();

    if (stmt.superclass != nullptr)
        endScope();

    currentClass = enclosingClass;
}

void Resolver::visitIfStmt(const If &stmt) {
    resolve(stmt.main_branch.condition);
    resolve(stmt.main_branch.statement);
    for (const auto &elif: stmt.elif_branches) {
        resolve(elif.condition);
        resolve(elif.statement);
    }
    if (stmt.else_branch != nullptr) {
        resolve(stmt.else_branch);
    }
}

void Resolver::visitWhileStmt(const While &stmt) {
    ++loop_nesting_level;
    resolve(stmt.condition);
    resolve(stmt.body);
    --loop_nesting_level;
}

void Resolver::visitFunctionStmt(shared_ptr<Function> stmt) {
    declare(stmt->functionName);
    define(stmt->functionName);
    resolveFunction(stmt, FUNCTION);
}

void Resolver::visitReturnStmt(const Return &stmt) {
    if (currentFunction == FUNCTION_NONE) {
        Error::addError(stmt.name, "Resolvetime Error. Cannot return from top-level code.");
        // lox::error(stmt.name.line, "Resolvetime Error. Cannot return from
        // top-level code.");
    }

    if (stmt.value != nullptr) {
        if (currentFunction == INITIALIZER) {
            Error::addError(
                stmt.name,
                "Resolvetime Error. Cannot return a value from an initializer."
            );
            // lox::error(stmt.name.line,
            //            "Resolvetime Error. Cannot return a value from an
            //            initializer.");
        }
        resolve(stmt.value);
    }
}

void Resolver::visitBreakStmt(const Break &stmt) {
    // If not in a nested loop, add a new error.
    if (loop_nesting_level == 0) {
        Error::addError(stmt.keyword, "Resolvetime Error. Can't break outside of a loop.");
        // lox::error(stmt.keyword.line, "Resolvetime Error. Can't break outside of
        // a loop.");
    }
}

void Resolver::visitContinueStmt(const Continue &stmt) {
    // If not in a nested loop, add a new error.
    if (loop_nesting_level == 0) {
        Error::addError(stmt.keyword, "Resolvetime Error. Can't break outside of a loop.");
        // lox::error(stmt.keyword.line, "Resolvetime Error. Can't break outside of
        // a loop.");
    }
}

void Resolver::beginScope() {
    map<string, bool> m;
    scopes.push_back(m);
}

void Resolver::endScope() { scopes.pop_back(); }

void Resolver::resolve(vector<shared_ptr<Stmt>> statements) {
    for (auto statement: statements) {
        resolve(statement);
    }
}

void Resolver::resolve(shared_ptr<Stmt> stmt) {
    stmt->accept(shared_from_this());
}

void Resolver::resolve(shared_ptr<Expr<Object>> expr) {
    expr->accept(shared_from_this());
}

void Resolver::declare(Token name) {
    if (scopes.empty())
        return;
    map<string, bool> scope = scopes.back();
    if (scope.find(name.lexeme) != scope.end()) {
        Error::addError(name, "Resolvetime Error. Variable with the name '" + name.lexeme + "' already exists in this scope");
        // lox::error(name.line,
        //            "Resolvetime Error. Variable with this name already declared
        //            in this scope.");
    }
    scope[name.lexeme] = false;
    scopes.pop_back();
    scopes.emplace_back(scope);
}

void Resolver::define(Token name) {
    if (scopes.empty())
        return;
    map<string, bool> scope = scopes.back();
    scope[name.lexeme] = true;
    scopes.pop_back();
    scopes.emplace_back(scope);
}

void Resolver::resolveLocal(shared_ptr<Expr<Object>> expr, Token name) {
    for (int i = scopes.size() - 1; i >= 0; i--) {
        auto searched = scopes[i].find(name.lexeme);
        if (searched != scopes[i].end()) {
            interpreter->resolve(expr, scopes.size() - 1 - i);
            return;
        }
    }
    // Not found. Assume it is global.
}

void Resolver::resolveFunction(shared_ptr<Function> function, FunctionType type) {
    FunctionType enclosingFunction = currentFunction;
    currentFunction = type;

    beginScope();
    for (auto param: function->params) {
        declare(param.first);
        define(param.first);
    }
    resolve(function->body);
    endScope();
    currentFunction = enclosingFunction;
}
