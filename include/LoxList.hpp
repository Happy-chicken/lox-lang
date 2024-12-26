#ifndef LIST_TYPE_HPP
#define LIST_TYPE_HPP
#include <algorithm>
#include <stdexcept>
#include <vector>
#include "LoxInstance.hpp"
class Object;

class LoxList
{
public:
    LoxList();

    explicit LoxList(std::vector<Object> values);

    size_t length() const noexcept;

    Object &at(int index);

    void append(const Object &value);

    Object pop() noexcept;

    void remove(int index);

private:
    std::vector<Object> values;
    size_t len = 0u;
};
#endif