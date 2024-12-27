#include <iostream>
#include <memory>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <variant>
#include <vector>

#include "../../include/BuiltInClass.hpp"
#include "../../include/BuiltInFun.hpp"
#include "../../include/BuiltInIo.hpp"
#include "../../include/Environment.hpp"
#include "../../include/Expr.hpp"
#include "../../include/Interpreter.hpp"
#include "../../include/Logger.hpp"
#include "../../include/LoxCallable.hpp"
#include "../../include/LoxClass.hpp"
#include "../../include/LoxFunction.hpp"
#include "../../include/LoxInstance.hpp"
#include "../../include/LoxList.hpp"
#include "../../include/RuntimeError.hpp"
#include "../../include/RuntimeException.hpp"
#include "../../include/Stmt.hpp"
#include "../../include/lox.hpp"

using std::shared_ptr;
using std::stod;
using std::to_string;
using std::vector;

bool endsWith(std::string str, std::string suffix) {
    if (str.length() < suffix.length()) {
        return false;
    }
    return str.substr(str.length() - suffix.length()) == suffix;
}

Interpreter::Interpreter()//: global_environment{globals.get()}
{
    // native function
    globals->define("clock", Object::make_obj(std::make_shared<ClockCallable>()));
    globals->define("print", Object::make_obj(std::make_shared<PrintCallable>()));
    globals->define("type", Object::make_obj(std::make_shared<TypeCallable>()));
    globals->define("input", Object::make_obj(std::make_shared<InputCallable>()));
    // native class
    globals->define("list", Object::make_class_obj(std::make_shared<ListClass>()));

    // environment = globals;
}

/// @brief interpret the list of statements
/// @param statements statements sequence that are generatede by the parser
void Interpreter::interpret(vector<shared_ptr<Stmt>> statements) {
    try {
        for (auto statement: statements) {
            execute(statement);
        }
    } catch (const RuntimeError &error) {
        Error::addRuntimeError(error);
        // lox::runtimeError(error);
    } catch (const ReturnError &) {
        //...
    }
}
// runner function
Object Interpreter::evaluate(shared_ptr<Expr<Object>> expr) {
    return expr->accept(shared_from_this());
}

void Interpreter::execute(shared_ptr<Stmt> stmt) {
    stmt->accept(shared_from_this());
}

void Interpreter::resolve(shared_ptr<Expr<Object>> expr, int depth) {
    locals[expr] = depth;
}

void Interpreter::executeBlock(vector<shared_ptr<Stmt>> statements, shared_ptr<Environment> env) {
    // shared_ptr<Environment> previous = environment;
    // environment = env;
    // * enter a new environment, enclosing env using RAII technique
    EnvironmentGuard env_guard{*this, env};
    for (auto statement: statements) {
        execute(statement);
    }
    // environment = previous;
}

// helper function
// string Interpreter::stringify(Object object) {
//   string text = object.toString();
//   string check_str = ".000000";
//   if (endsWith(text, check_str)) {
//     return text.erase(text.size() - check_str.size());
//   }
//   return text;
// }

bool Interpreter::isTruthy(Object object) {
    if (object.data.index() == 3) {
        return false;
    }
    if (object.data.index() == 2) {
        return std::get<bool>(object.data);
    }
    // if (object.type == Object::Object_nil) {
    //   return false;
    // }
    // if (object.type == Object::Object_bool) {
    //   return object.boolean;
    // }
    return true;
}

bool Interpreter::isEqual(Object a, Object b) {
    if (a.data.index() == 3 && b.data.index() == 3) {
        return true;
    }
    if (a.data.index() == 3) {
        return false;
    }
    if (a.data.index() == b.data.index()) {
        switch (a.data.index()) {
            case 0:
                return std::get<std::string>(a.data) == std::get<std::string>(b.data);
            case 1:
                return std::get<double>(a.data) == std::get<double>(b.data);
            case 2:
                return std::get<bool>(a.data) == std::get<bool>(b.data);
            case 3:
                return true;
            case 8:
                return std::get<int>(a.data) == std::get<int>(b.data);
            default:
                return false;
        }
    } else {
        return false;
    }
}

void Interpreter::checkNumberOperand(Token operation, Object operand) {
    if (operand.data.index() == 1 || operand.data.index() == 8)// operand.type == Object::Object_num
        return;
    throw RuntimeError(operation, "Runtime Error. Operand must be a number.");
}

void Interpreter::checkNumberOperands(Token operation, Object left, Object right) {
    // left.type == Object::Object_num && right.type == Object::Object_num
    if ((left.data.index() == 1 && right.data.index() == 1) || (left.data.index() == 8 && right.data.index() == 8))
        return;
    throw RuntimeError(operation, "Runtime Error. Operands must be a number.");
}

Object Interpreter::lookUpVariable(Token name, shared_ptr<Expr<Object>> expr) {
    if (locals.find(expr) != locals.end()) {
        auto distance = locals.at(expr);
        return environment->getAt(distance, name.lexeme);
    }
    return globals->get(name);
}

Object Interpreter::visitLiteralExpr(shared_ptr<Literal<Object>> expr) {
    switch (expr->value.data.index()) {
        case 1:
            return Object::make_obj(std::get<double>(expr->value.data));
        case 2:
            return Object::make_obj(std::get<bool>(expr->value.data));
        case 3:
            return Object::make_nil_obj();
        case 8:
            return Object::make_obj(std::get<int>(expr->value.data));

        default:
            return Object::make_obj(std::get<std::string>(expr->value.data));
    }
}

Object Interpreter::visitGroupingExpr(shared_ptr<Grouping<Object>> expr) {
    return evaluate(expr->expression);
}

Object Interpreter::visitUnaryExpr(shared_ptr<Unary<Object>> expr) {
    Object right = evaluate(expr->right);
    switch (expr->operation.type) {
        case BANG:
            return Object::make_obj(!isTruthy(right));
        case MINUS:
            return Object::make_obj(-stod(right.toString()));
        default:
            // Unreachable.
            return Object::make_nil_obj();
    }
}

Object Interpreter::visitBinaryExpr(shared_ptr<Binary<Object>> expr) {
    Object left = evaluate(expr->left);
    Object right = evaluate(expr->right);
    bool result_bool = false;
    string result_str = "";
    double result_num = 0;
    int result_i32num = 0;
    bool intflag = left.data.index() == 8 || right.data.index() == 8;
    // Object foo;
    switch (expr->operation.type) {
        case GREATER:
            checkNumberOperands(expr->operation, left, right);
            result_bool = intflag ? std::get<int>(left.data) > std::get<int>(right.data) : std::get<double>(left.data) > std::get<double>(right.data);
            return Object::make_obj(result_bool);
        case GREATER_EQUAL:
            checkNumberOperands(expr->operation, left, right);
            result_bool = intflag ? std::get<int>(left.data) >= std::get<int>(right.data) : std::get<double>(left.data) >= std::get<double>(right.data);
            return Object::make_obj(result_bool);
        case LESS:
            checkNumberOperands(expr->operation, left, right);
            result_bool = intflag ? std::get<int>(left.data) < std::get<int>(right.data) : std::get<double>(left.data) < std::get<double>(right.data);
            return Object::make_obj(result_bool);
        case LESS_EQUAL:
            checkNumberOperands(expr->operation, left, right);
            result_bool = intflag ? std::get<int>(left.data) <= std::get<int>(right.data) : std::get<double>(left.data) <= std::get<double>(right.data);
            return Object::make_obj(result_bool);
        case MINUS:
            checkNumberOperand(expr->operation, right);
            if (left.data.index() == 1 && right.data.index() == 1) {
                result_num = std::get<double>(left.data) - std::get<double>(right.data);
                return Object::make_obj(result_num);
            }
            // int
            if (left.data.index() == 8 && right.data.index() == 8) {
                result_i32num = std::get<int>(left.data) - std::get<int>(right.data);
                return Object::make_obj(result_i32num);
            }
        case PLUS:
            // left.type == Object::Object_num && right.type == Object::Object_num
            if (left.data.index() == 1 && right.data.index() == 1) {
                result_num = std::get<double>(left.data) + std::get<double>(right.data);
                return Object::make_obj(result_num);
            }
            // int
            if (left.data.index() == 8 && right.data.index() == 8) {
                result_i32num = std::get<int>(left.data) + std::get<int>(right.data);
                return Object::make_obj(result_i32num);
            }
            // left.type == Object::Object_str && right.type == Object::Object_str
            if (left.data.index() == 0 && right.data.index() == 0) {
                result_str = left.toString() + right.toString();
                return Object::make_obj(result_str);
            }
            throw RuntimeError(
                expr->operation,
                "Runtime Error. Operands must be two numbers or two strings."
            );
        case SLASH:
            checkNumberOperands(expr->operation, left, right);
            result_num = std::get<double>(left.data) / std::get<double>(right.data);
            return Object::make_obj(result_num);
        case STAR:
            checkNumberOperands(expr->operation, left, right);
            result_num = std::get<double>(left.data) * std::get<double>(right.data);
            return Object::make_obj(result_num);
        case BANG_EQUAL:
            return Object::make_obj(!isEqual(left, right));
        case EQUAL_EQUAL:
            return Object::make_obj(isEqual(left, right));
        case CARAT: // TODO square
        case MODULO:// TODO mod
            checkNumberOperands(expr->operation, left, right);
            result_num = static_cast<int>(std::get<double>(left.data)) %
                         static_cast<int>(std::get<double>(right.data));
            return Object::make_obj(result_num);
        case BACKSLASH:// TODO floor divide
        default:
            return Object::make_nil_obj();
    }
}

Object Interpreter::visitAssignExpr(shared_ptr<Assign<Object>> expr) {
    Object value = evaluate(expr->value);

    // environment->assign(expr->name, value);
    auto distance = locals.find(expr);
    if (distance != locals.end()) {
        environment->assignAt(distance->second, expr->name, value);
    } else {
        globals->assign(expr->name, value);
    }
    return value;
}

Object Interpreter::visitLogicalExpr(shared_ptr<Logical<Object>> expr) {
    Object left = evaluate(expr->left);
    if (expr->operation.type == OR) {
        if (isTruthy(left))
            return left;
    } else {
        if (!isTruthy(left))
            return left;
    }
    return evaluate(expr->right);
}

Object Interpreter::visitIncrementExpr(shared_ptr<Increment<Object>> expr) {
    // get current variable value that is being incremented
    Object old_value = lookUpVariable(expr->identifier, expr);
    // old_value.type == Object::Object_type::Object_num
    if (old_value.data.index() == 1) {
        throw RuntimeError(expr->identifier, "Runtime Error. Cannot increment a non integer type '" + expr->identifier.lexeme + "'.");
    }
    // add one, temp var to be returned later
    const Object new_value =
        Object::make_obj(std::get<double>(old_value.data) + 1);
    // update pointer's value
    old_value.data = std::get<double>(old_value.data) + 1;
    // If the expression is a postfix increment, return the old value
    // otherwise return the new value.
    return expr->type == Increment<Object>::Type::POSTFIX ? old_value : new_value;
}

Object Interpreter::visitDecrementExpr(shared_ptr<Decrement<Object>> expr) {
    // get current variable value that is being incremented
    Object old_value = lookUpVariable(expr->identifier, expr);
    // old_value.type == Object::Object_type::Object_num
    if (old_value.data.index() == 1) {
        throw RuntimeError(expr->identifier, "Runtime Error. Cannot increment a non integer type '" + expr->identifier.lexeme + "'.");
    }
    // add one, temp var to be returned later
    const Object new_value =
        Object::make_obj(std::get<double>(old_value.data) + 1);
    // update pointer's value
    old_value.data = std::get<double>(old_value.data) + 1;
    // If the expression is a postfix increment, return the old value
    // otherwise return the new value.
    return expr->type == Decrement<Object>::Type::POSTFIX ? old_value : new_value;
}

/// @brief visit a list expression, define a list, like a =[]
/// @param expr list expression
/// @return
Object Interpreter::visitListExpr(shared_ptr<List<Object>> expr) {
    auto list = std::make_shared<LoxList>();
    // evaluate the each element in the list
    for (const auto &item: expr->items) {
        assert(item);
        list->append(evaluate(item));
    }
    return Object::make_obj(list);
}

/// @brief visit a subscript expression, call a list, like a[]
/// @param expr subscript expression
/// @return
Object Interpreter::visitSubscriptExpr(shared_ptr<Subscript<Object>> expr) {
    // get the pointer to the list associated with the identifier
    Object iden_ptr = lookUpVariable(expr->identifier, expr);
    // check if the identifier is a list, if not, throw an error
    // iden_ptr.type != Object::Object_type::Object_list
    if (iden_ptr.data.index() != 4) {
        throw RuntimeError(expr->identifier, "Runtime Error. Object " + expr->identifier.lexeme + "can not be subsripted");
    }
    // Dereference the pointer to access the underlying objects pointer.

    auto list = std::get<shared_ptr<LoxList>>(iden_ptr.data);

    // Evaluate the index expression.
    Object index = evaluate(expr->index);
    double index_cast = 0;

    // Refers to the size of the original list object.
    size_t object_size = 0u;

    // Anything else than numbers for indexes are not allowed.
    // index.type == Object::Object_type::Object_num
    if (index.data.index() == 1) {
        index_cast = std::get<double>(index.data);
    } else {
        throw RuntimeError(expr->identifier, "Runtime Error. Indices must be integers.");
    }
    // Throw an error if index is not an integer.
    if (static_cast<int>(index_cast) != index_cast) {
        throw RuntimeError(expr->identifier, "Runtime Error. Indices must be integers.");
    }

    try {
        object_size = list->length();

        // ? something superfluous, beacuse "at" has allowed this manner
        if (index_cast < 0)// allow this manner
        {
            index_cast = static_cast<int>(list->length()) + index_cast;
        }
        // If value is associated with the subscript expression, new value will be
        // assigned to the corresponding index.
        if (expr->value) {
            list->at(index_cast) = evaluate(expr->value);
        }
        return list->at(index_cast);
    } catch (const std::out_of_range &e) {
        throw RuntimeError(expr->identifier, "RunTime Error. Index out of range. Index is " + to_string(static_cast<int>(index_cast)) + " but object size is " + to_string(object_size));
    }
}

// TODO
// Object visitLambdaExpr(shared_ptr<Lambda<Object>> expr)
// {
//     return Object::make_nil_obj();
// }

Object Interpreter::visitVariableExpr(shared_ptr<Variable<Object>> expr) {
    return lookUpVariable(expr->name, expr);
}

Object Interpreter::visitCallExpr(shared_ptr<Call<Object>> expr) {
    // search the callee in the environment and return it(function, class,
    // instance)
    Object callee = evaluate(expr->callee);

    vector<Object> arguments;
    arguments.reserve(expr->arguments.size());
    for (auto argument: expr->arguments) {
        arguments.push_back(evaluate(argument));
    }
    // callee.type != Object::Object_fun &&callee.type !=
    // Object::Object_class

    if (callee.data.index() != 5 && callee.data.index() != 7) {
        throw RuntimeError(expr->paren, "Runtime Error. Can only call functions and classes.");
    }

    shared_ptr<LoxCallable> callable;

    if (std::holds_alternative<shared_ptr<LoxCallable>>(
            callee.data
        )) {// callee.type == Object::Object_fun
        callable = std::get<shared_ptr<LoxCallable>>(callee.data);
        auto func = std::get<shared_ptr<LoxCallable>>(callee.data).get();
        if (dynamic_cast<PrintCallable *>(std::move(func))) {
            callable = std::make_shared<PrintCallable>(arguments.size());
        } else if (dynamic_cast<ClockCallable *>(std::move(func))) {
            callable = std::make_shared<ClockCallable>();
        } else if (dynamic_cast<TypeCallable *>(std::move(func))) {
            callable = std::make_shared<TypeCallable>(arguments.size());
        } else if (dynamic_cast<InputCallable *>(std::move(func))) {
            callable = std::make_shared<InputCallable>();
        }
        if (auto ptr_list_len = dynamic_cast<ListLenMethods *>(std::move(func))) {
            return ptr_list_len->call(shared_from_this(), arguments);
        }
    }
    // callee.type == Object::Object_class
    if (callee.data.index() == 7) {
        callable = std::get<shared_ptr<LoxClass>>(callee.data);
        if (dynamic_cast<ListClass *>(callable.get())) {
            return callable->call(shared_from_this(), arguments);
        }
    }
    if (arguments.size() != callable->arity()) {
        throw RuntimeError(expr->paren, "Runtime Error. Expected " + to_string(callable->arity()) + " arguments but got " + to_string(arguments.size()) + ".");
    }
    return callable->call(shared_from_this(), arguments);
}

Object Interpreter::visitGetExpr(shared_ptr<Get<Object>> expr) {
    Object object = evaluate(expr->object);
    // object.type == Object::Object_instance
    if (object.data.index() == 6) {
        return std::get<shared_ptr<LoxInstance>>(object.data)->get(expr->name);
    }

    throw RuntimeError(expr->name, "Runtime Error. Only instances have properties.");
}

Object Interpreter::visitSetExpr(shared_ptr<Set<Object>> expr) {
    Object object = evaluate(expr->object);
    // object.type != Object::Object_instance
    if (object.data.index() != 6) {
        throw RuntimeError(expr->name, "Runtime Error. Only instances have fields.");
    }

    Object value = evaluate(expr->value);
    std::get<shared_ptr<LoxInstance>>(object.data)->set(expr->name, value);
    return value;
}

Object Interpreter::visitThisExpr(shared_ptr<This<Object>> expr) {
    return lookUpVariable(expr->keyword, expr);
}

Object Interpreter::visitSuperExpr(shared_ptr<Super<Object>> expr) {
    int distance = locals[expr];
    Object superclass = environment->getAt(distance, "super");

    // "this" is always one level nearer than "super"'s environment.
    Object instance = environment->getAt(distance - 1, "this");

    shared_ptr<LoxFunction> method =
        std::get<shared_ptr<LoxClass>>(superclass.data)
            ->findMethod(expr->method.lexeme);

    if (method == nullptr) {
        throw RuntimeError(expr->method, "Runtime Error. Undefined property '" + expr->method.lexeme + "'.");
    }

    shared_ptr<LoxFunction> binded_method =
        method->bind(std::get<shared_ptr<LoxInstance>>(instance.data));
    return Object::make_obj(binded_method);
}

void Interpreter::visitExpressionStmt(const Expression &stmt) {
    evaluate(stmt.expression);
}

void Interpreter::visitPrintStmt(const Print &stmt) {
    evaluate(stmt.expression);
    // Object value = evaluate(stmt.expression);
    // cout << stringify(value) << endl;
}

void Interpreter::visitReturnStmt(const Return &stmt) {
    Object value;
    if (stmt.value != nullptr)
        value = evaluate(stmt.value);
    throw ReturnError(value);
}
void Interpreter::visitBreakStmt(const Break &stmt) {
    throw BreakException(stmt.keyword);
}

void Interpreter::visitContinueStmt(const Continue &stmt) {
    throw ContinueException(stmt.keyword);
}

void Interpreter::visitVarStmt(const Var &stmt) {
    Object value = Object::make_nil_obj();
    if (stmt.initializer != nullptr) {
        value = evaluate(stmt.initializer);
    }
    environment->define(stmt.name.lexeme, value);
}

void Interpreter::visitBlockStmt(const Block &stmt) {
    shared_ptr<Environment> env = std::make_shared<Environment>(environment);
    executeBlock(stmt.statements, env);
}

void Interpreter::visitClassStmt(const Class &stmt) {
    Object superclass = Object::make_nil_obj();
    if (stmt.superclass != nullptr) {
        superclass = evaluate(stmt.superclass);
        // superclass.type != Object::Object_class
        if (superclass.data.index() != 7) {
            throw RuntimeError(stmt.superclass->name, "Runtime Error. Superclass must be a class.");
        }
    }

    environment->define(stmt.name.lexeme, Object::make_nil_obj());

    if (stmt.superclass != nullptr) {
        environment = std::make_shared<Environment>(environment);
        environment->define("super", superclass);
    }

    map<string, shared_ptr<LoxFunction>> methods;
    for (auto method: stmt.methods) {
        bool is_init = method->name.lexeme == "init";
        shared_ptr<LoxFunction> function =
            std::make_shared<LoxFunction>(method, environment, is_init);

        methods[method->name.lexeme] = function;
    }

    auto klass = std::make_shared<LoxClass>(
        stmt.name.lexeme,
        superclass.data.index() == 7
            ? std::get<shared_ptr<LoxClass>>(superclass.data)
            : nullptr,
        methods
    );

    if (superclass.data.index() != 3) {
        environment = environment->enclosing;
    }

    environment->assign(stmt.name, Object::make_obj(klass));
}

void Interpreter::visitWhileStmt(const While &stmt) {
    while (isTruthy(evaluate(stmt.condition))) {
        try {
            execute(stmt.body);
        }
        // * use try catch mechinism in C++ to realize break and continue
        // If a continue statement is encountered, continue to the next iteration.
        catch (const ContinueException &) {
            // Do nothing.
        }
        // If a break statement is encountered, exit the loop.
        catch (const BreakException &) {
            return;
        }
    }
}

void Interpreter::visitIfStmt(const If &stmt) {
    // ! it cannot interprete nested if-else expression
    if (isTruthy(evaluate(stmt.main_branch.condition))) {
        execute(stmt.main_branch.statement);
    }
    for (auto &else_if: stmt.elif_branches) {
        if (isTruthy(evaluate(else_if.condition))) {
            execute(else_if.statement);
        }
    }
    if (stmt.else_branch != nullptr) {
        execute(stmt.else_branch);
    }
}

void Interpreter::visitFunctionStmt(shared_ptr<Function> stmt) {
    shared_ptr<LoxFunction> function =
        std::make_shared<LoxFunction>(stmt, environment, false);
    Object obj = Object::make_obj(function);
    environment->define(stmt->name.lexeme, obj);
}

// The EnvironmentGuard class is used to manage the interpreter's environment
// stack. It follows the RAII technique, which means that when an instance of
// the class is created, a copy of the current environment is stored, and the
// current environment is moved to the new one. If a runtime error is
// encountered and the interpreter needs to unwind the stack and return to the
// previous environment, the EnvironmentGuard class's destructor is called,
// which swaps the resources back to the previous environment.
Interpreter::EnvironmentGuard::EnvironmentGuard(
    Interpreter &interpreter, std::shared_ptr<Environment> enclosing_env
)
    : interpreter{interpreter}, previous_env{interpreter.environment} {
    interpreter.environment = std::move(enclosing_env);
}

Interpreter::EnvironmentGuard::~EnvironmentGuard() {
    interpreter.environment = std::move(previous_env);
}
