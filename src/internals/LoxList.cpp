#include "../../include/LoxList.hpp"
#include "../../include/RuntimeError.hpp"

// constrcutor
LoxList::LoxList() : len{0} {}

LoxList::LoxList(std::vector<Object> values_) : values{std::move(values_)} {
  len = this->values.size();
}

/// @brief calculate the list's length
/// @return length of list
size_t LoxList::length() const noexcept { return len; }

/// @brief calcualte the slice
/// @param index index of the list you want
/// @return
Object &LoxList::at(int index) {
  // * support for minus index
  return index < 0 ? values[len + index] : values[index];
}

/// @brief add an element at the end
/// @param value the value you want to add
void LoxList::append(const Object &value) {
  values.push_back(value);
  len += 1;
}

/// @brief pop the element at the end
/// @return the end value
Object LoxList::pop() noexcept {
  const Object value = values.back();
  values.pop_back();
  len -= 1;

  return value;
}

/// @brief remove the value at the given index
/// @param index
void LoxList::remove(int index) {
  if (index < 0) {
    values.erase(values.end() - 1 + index);
  } else {
    values.erase(values.begin() + index);
  }

  len -= 1;
}