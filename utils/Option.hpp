#pragma once

#include <stdexcept>
#include <string>

template <typename T>

class Option {
protected:
  T value;
  bool has_value;
  bool checked;

public:
  Option() : has_value(false), checked(false) {}
  Option(T value) : value(value), has_value(true), checked(false) {}
  Option(bool is_empty) : has_value(!is_empty), checked(false) {}
  Option(Option &other) {
    this->value = other.value;
    this->has_value = other.has_value;
    this->checked = other.checked;
  }
  T safe_unwrap(void) {
    if (checked == false)
      throw std::invalid_argument(
          "Tried to safe_unwrap without checking for none");

    if (has_value == false)
      throw std::runtime_error("Unwraped option that contained nothing");
    else
      return value;
  }
  T unwrap(void) {
    if (checked == true && has_value == true)
      return this->value;

    if (has_value == false)
      throw std::runtime_error("Unwraped option that contained nothing");
    else
      return value;
  }
  bool check(void) {
    this->checked = true;
    return (this->has_value);
  }
  T unwrap_or(T &other) {
    if (checked == true && has_value == true)
      return this->value;

    if (has_value == false)
      return (other);
    else
      return value;
  }
  T expect(const std::string &error_mesage) {
    if (checked == true && has_value == true)
      return this->value;

    if (has_value == false)
      throw std::runtime_error(error_mesage);
    else
      return value;
  }
  ~Option() {}
};
