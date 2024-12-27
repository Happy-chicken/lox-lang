#ifndef ENVIRONMENT_HPP_
#define ENVIRONMENT_HPP_

#include "Logger.hpp"
#include "Token.hpp"
#include <llvm/IR/Use.h>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

using std::shared_ptr;
using std::string;
using std::unordered_map;

class Environment : public std::enable_shared_from_this<Environment> {
public:
    Environment() = default;
    explicit Environment(shared_ptr<Environment> enclosing);
    Environment(shared_ptr<Environment> enclosing, std::map<string, llvm::Value *> llvmRecord) : enclosing(enclosing), llvmRecord_(llvmRecord){};

    void define(string name, Object value);
    llvm::Value *define(string name, llvm::Value *value);

    void assign(Token name, Object value);
    void assignAt(int distance, Token name, Object value);

    Object get(Token name);
    Object getAt(int distance, string name);
    llvm::Value *lookup(const string &name);

    unordered_map<string, Object> getValues() const { return values; }
    shared_ptr<Environment> ancestor(int distance);
    shared_ptr<Environment> enclosing;// parent environment link(parent_)

private:
    unordered_map<string, Object> values;
    std::map<string, llvm::Value *> llvmRecord_;

    std::shared_ptr<Environment> resolve(const string &name) {
        if (llvmRecord_.count(name) != 0) {
            return shared_from_this();
        }
        if (enclosing == nullptr) {
            Error::ErrorLogMessage() << "Undefined variable '" << name << "'.";
        }
        return enclosing->resolve(name);
    }
};

#endif// ENVIRONMENT_HPP_
