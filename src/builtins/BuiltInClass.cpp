#include "../../include/BuiltInClass.hpp"
#include "../../include/LoxList.hpp"
#include "../../include/RuntimeError.hpp"
#include "../../include/Stmt.hpp"
#include <iostream>
#include <memory>
#include <string>

// ------------------------------------------------------------------------------------------
ListClass::ListClass() : LoxClass("list", nullptr, {}) {
    this->methods["len"] = std::make_shared<ListLenMethods>(
        std::make_shared<Environment>(),
        std::make_shared<Function>(Token(), vector<std::pair<Token, string>>{}, vector<shared_ptr<Stmt>>{}, Token())
    );
    this->methods["append"] = std::make_shared<ListAppendMethods>(
        std::make_shared<Environment>(),
        std::make_shared<Function>(Token(), vector<std::pair<Token, string>>{{Token(), ""}}, vector<shared_ptr<Stmt>>{}, Token())
    );
}

ListClass::ListClass(map<string, shared_ptr<LoxFunction>> methods_)
    : LoxClass("list", nullptr, std::move(methods_)) {}

Object ListClass::call(shared_ptr<Interpreter> interpreter, std::vector<Object> args) {
    //   return
    //   Object::make_list_obj(std::make_shared<LoxList>(std::move(args)));
    this->list = args;
    auto list_instance = std::make_shared<ListInstance>(*this);
    return Object::make_instance_obj(list_instance);
}
// ------------------------------------------------------------------------------------------
ListInstance::ListInstance(ListClass klass) : LoxInstance(klass) {
    this->fields.emplace("type", Object::make_obj("List-instance"));
    this->fields.emplace(
        "value", Object::make_obj(std::make_shared<LoxList>(klass.getList()))
    );
}
// ------------------------------------------------------------------------------------------
ListLenMethods::ListLenMethods(shared_ptr<Environment> closure_, shared_ptr<Function> declaration_)
    : LoxFunction(declaration_, closure_, false) {}

Object ListLenMethods::call(shared_ptr<Interpreter> interpreter, std::vector<Object> args) {
    size_t length =
        std::get<shared_ptr<LoxList>>(std::get<shared_ptr<LoxInstance>>(
                                          this->closure->getValues()["this"].data
                                      )
                                          ->fields["value"]
                                          .data)
            ->length();
    return Object::make_obj(double(length));
}

shared_ptr<ListLenMethods>
ListLenMethods::bind(shared_ptr<ListInstance> instance) {
    auto environment = std::make_shared<Environment>(closure);
    environment->define("this", Object::make_instance_obj(instance));
    return std::make_shared<ListLenMethods>(environment, declaration);
}
// ------------------------------------------------------------------------------------------
ListAppendMethods::ListAppendMethods(shared_ptr<Environment> closure_, shared_ptr<Function> declaration_)
    : LoxFunction(declaration_, closure_, false) {}

Object ListAppendMethods::call(shared_ptr<Interpreter> interpreter, std::vector<Object> args) {
    std::get<shared_ptr<LoxList>>(
        std::get<shared_ptr<LoxInstance>>(this->closure->getValues()["this"].data)
            ->fields["value"]
            .data
    )
        ->append(args[0]);
    return Object::make_nil_obj();
}

shared_ptr<ListAppendMethods>
ListAppendMethods::bind(shared_ptr<ListInstance> instance) {
    auto environment = std::make_shared<Environment>(closure);
    environment->define("this", Object::make_instance_obj(instance));
    return std::make_shared<ListAppendMethods>(environment, declaration);
}
// ------------------------------------------------------------------------------------------