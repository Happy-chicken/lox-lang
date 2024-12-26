// Copyright 2020.2.23 <Copyright hulin>

#include <string>
#include <memory>
#include <vector>

#include "../../include/LoxClass.hpp"
#include "../../include/LoxInstance.hpp"
#include "../../include/Interpreter.hpp"
#include "../../include/LoxFunction.hpp"
#include "../../include/Token.hpp"

using std::shared_ptr;
using std::string;
using std::vector;

LoxClass::LoxClass(
    string name_,
    shared_ptr<LoxClass> superclass_,
    map<string, shared_ptr<LoxFunction>> methods_) : name(name_), superclass(superclass_), methods(methods_) {}

Object LoxClass::call(
    shared_ptr<Interpreter> interpreter,
    vector<Object> arguments)
{
    auto instance = std::make_shared<LoxInstance>(*this);
    // auto instance = shared_ptr<LoxInstance>(new LoxInstance(*this));
    shared_ptr<LoxFunction> initializer = findMethod("init");
    if (initializer != nullptr)
    {
        initializer->bind(instance)->call(interpreter, arguments);
    }
    return Object::make_instance_obj(instance);
}

size_t LoxClass::arity()
{
    shared_ptr<LoxFunction> initializer = findMethod("init");
    if (initializer == nullptr)
        return 0;
    return initializer->arity();
}
string LoxClass::toString()
{
    return "<class " + name + ">";
}

shared_ptr<LoxFunction> LoxClass::findMethod(string name)
{
    auto searched = methods.find(name);
    if (searched != methods.end())
    {
        return searched->second;
    }
    if (superclass != nullptr)
    {
        return superclass->findMethod(name);
    }
    return nullptr;
}