#ifndef LOXINSTANCE_HPP_
#define LOXINSTANCE_HPP_

#include <string>
#include <memory>
#include <map>
#include "LoxClass.hpp"
#include "Token.hpp"

using std::map;
using std::string;

class LoxInstance : public std::enable_shared_from_this<LoxInstance>
{
public:
    LoxClass klass;
    map<string, Object> fields;

    Object get(Token name);
    void set(Token name, Object value);
    explicit LoxInstance(LoxClass klass_);
    string toString();
};

#endif // LOXINSTANCE_HPP_
