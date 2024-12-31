// Copyright 2020 <Copyright hulin>

#include "../../include/LoxFunction.hpp"
#include "../../include/Environment.hpp"
#include "../../include/Interpreter.hpp"
#include "../../include/LoxInstance.hpp"
#include "../../include/RuntimeException.hpp"
#include "../../include/Stmt.hpp"
#include "../../include/Token.hpp"
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using std::shared_ptr;
using std::string;
using std::vector;

LoxFunction::LoxFunction(shared_ptr<Function> declaration_, shared_ptr<Environment> closure_, bool isInitializer_)
    : declaration(declaration_), closure(closure_),
      isInitializer(isInitializer_) {}

size_t LoxFunction::arity() { return declaration->params.size(); }

Object LoxFunction::call(shared_ptr<Interpreter> interpreter, vector<Object> arguments) {
    auto environment = std::make_shared<Environment>(closure);
    // shared_ptr<Environment> environment(new Environment(closure));

    for (size_t i = 0; i < declaration->params.size(); i++) {
        environment->define(declaration->params[i].first.lexeme, arguments[i]);
    }
    try {
        interpreter->executeBlock(declaration->body, environment);
    } catch (ReturnError const &returnValue) {
        if (isInitializer) {
            return closure->getAt(0, "this");
        }
        return returnValue.getReturnValue();
    }
    if (isInitializer) {
        return closure->getAt(0, "this");
    }
    return Object::make_nil_obj();
}

string LoxFunction::toString() {
    return "<fn " + declaration->functionName.lexeme + ">";
}

shared_ptr<LoxFunction> LoxFunction::bind(shared_ptr<LoxInstance> instance) {
    auto environment = std::make_shared<Environment>(closure);
    environment->define("this", Object::make_instance_obj(instance));
    return std::make_shared<LoxFunction>(declaration, environment, isInitializer);
}