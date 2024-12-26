#include "../../include/BuiltInIo.hpp"
#include "../../include/BuiltInFun.hpp"
#include "../../include/LoxClass.hpp"
#include "../../include/LoxInstance.hpp"
#include "../../include/LoxList.hpp"
#include <memory>
#include <utility>
#include <variant>
using std::cin;

// Native print
size_t PrintCallable::arity() { return aritys; }

Object PrintCallable::call(shared_ptr<Interpreter> interpreter, std::vector<Object> args) {
    std::stringstream stream;
    for (const auto &arg: args) {
        stream << stringify(arg, stream) << ' ';
    }
    std::cout << stream.str() << '\n';
    return Object::make_nil_obj();
}

std::string PrintCallable::toString() { return "<native fn: print>"; }

// Native input
size_t InputCallable::arity() { return aritys; }

Object InputCallable::call(shared_ptr<Interpreter> interpreter, vector<Object> args) {
    std::cout << "Enter input: ";
    std::stringstream instream;
    std::string input;
    getline(cin, input);

    return Object::make_obj(input);
}

string InputCallable::toString() { return "<native fn: input>"; }

// Native print
std::string stringify(const Object &item, std::stringstream &stream) {
    // item.type == Object::Object_type::Object_bool
    if (std::holds_alternative<bool>(item.data))
        return std::get<bool>(item.data) ? "true" : "false";

    // if (item.type == Object::Object_type::Object_str)
    if (std::holds_alternative<std::string>(item.data)) {
        return std::get<std::string>(item.data);
    }
    // item.type == Object::Object_type::Object_num
    if (std::holds_alternative<double>(item.data)) {
        double num = std::get<double>(item.data);
        auto num_as_string = std::to_string(std::move(num));
        num_as_string.erase(num_as_string.find_last_not_of('0') + 1, std::string::npos);
        num_as_string.erase(num_as_string.find_last_not_of('.') + 1, std::string::npos);
        return num_as_string;
    }
    // item.type == Object::Object_type::Object_fun
    if (item.data.index() == 5)
        return std::get<shared_ptr<LoxCallable>>(item.data)->toString();
    // item.type == Object::Object_type::Object_class
    if (item.data.index() == 7)
        return std::get<shared_ptr<LoxClass>>(item.data)->toString();
    // item.type == Object::Object_type::Object_instance
    if (item.data.index() == 6) {
        return std::get<shared_ptr<LoxInstance>>(item.data)->toString();
    }
    // item.type == Object::Object_type::Object_list
    if (std::holds_alternative<shared_ptr<LoxList>>(item.data)) {
        auto items = std::get<shared_ptr<LoxList>>(item.data);
        stream << "[";
        auto len = items->length();
        for (size_t i = 0u; i < len; ++i) {
            stream << stringify(items->at(static_cast<int>(i)), stream);
            stream << ",";
        }
        stream.seekp(-1, std::ios_base::end);
        stream << "]";
        return {};
    }

    return "nil";
}
