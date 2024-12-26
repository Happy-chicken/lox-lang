#ifndef BUILTIN_CLASS_HPP
#define BUILTIN_CLASS_HPP
#include "LoxClass.hpp"
#include "LoxInstance.hpp"
#include "LoxList.hpp"
#include <memory>
#include <string>
#include <vector>
class ListClass : public LoxClass {
public:
    ListClass();

    explicit ListClass(map<string, shared_ptr<LoxFunction>> methods_);

    Object call(shared_ptr<Interpreter> interpreter, vector<Object> args);

    std::vector<Object> getList() const { return list; }

private:
    std::vector<Object> list{};
};

class ListInstance : public LoxInstance {
public:
    explicit ListInstance(ListClass klass_);
    // Object get(Token name);
};

class ListLenMethods : public LoxFunction {
public:
    explicit ListLenMethods(shared_ptr<Environment> closure_, shared_ptr<Function> declaration_);
    Object call(shared_ptr<Interpreter> interpreter, vector<Object> args);
    shared_ptr<ListLenMethods> bind(shared_ptr<ListInstance> instance);
};

class ListAppendMethods : public LoxFunction {
public:
    explicit ListAppendMethods(shared_ptr<Environment> closure_, shared_ptr<Function> declaration_);
    Object call(shared_ptr<Interpreter> interpreter, vector<Object> args);
    shared_ptr<ListAppendMethods> bind(shared_ptr<ListInstance> instance);
};
#endif// BUILTIN_CLASS_HPP
