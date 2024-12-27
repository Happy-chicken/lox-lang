
#include <map>
#include <memory>
#include <stdexcept>
#include <string>

#include "../../include/Environment.hpp"
#include "../../include/RuntimeError.hpp"

using std::map;
using std::string;

Environment::Environment(shared_ptr<Environment> enclosing_) : enclosing(enclosing_) {}

void Environment::define(string name, Object value) {
    values[name] = value;
}

// llvm environment
llvm::Value *Environment::define(string name, llvm::Value *value) {
    llvmRecord_[name] = value;
    return value;
}

Object Environment::get(Token name) {
    if (values.find(name.lexeme) != values.end()) {
        return values.at(name.lexeme);
    }
    if (enclosing) {
        return enclosing->get(name);
    }
    throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
}

Object Environment::getAt(int distance, string name) {
    return ancestor(distance)->values[name];
}

llvm::Value *Environment::lookup(const string &name) {
    return resolve(name)->llvmRecord_[name];
}

void Environment::assign(Token name, Object value) {
    if (values.find(name.lexeme) != values.end()) {
        values[name.lexeme] = value;
        return;
    }
    if (enclosing != nullptr) {
        enclosing->assign(name, value);
        return;
    }

    throw RuntimeError(name, "Undefined variable '" + name.lexeme + "'.");
}

void Environment::assignAt(int distance, Token name, Object value) {
    ancestor(distance)->values[name.lexeme] = value;
}

shared_ptr<Environment> Environment::ancestor(int distance) {
    shared_ptr<Environment> environment = shared_from_this();
    for (int i = 0; i < distance; i++) {
        environment = environment->enclosing;
    }
    return environment;
}