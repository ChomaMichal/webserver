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
  Option(bool is_empty) : has_value(false), checked(false) {}
  Option(Option &other) {
    this->value = other.value;
    this->has_value = other.has_value;
    this->checked = other.checked;
  }

  // will throw a runtime_error if it doesn't have vaule
  // if DEBUG is defined will throw invalid_argument if is not checked
  T unwrap(void) {
#ifdef DEBUG
    if (checked == false)
      throw std::invalid_argument(
          "Tried to safe_unwrap without checking for none");

#endif
    if (has_value == false)
      throw std::runtime_error("Unwraped option that contained nothing");
    else
      return value;
  }
  // returns true is there is value
  bool is_some(void) {
    this->checked = true;
    return (this->has_value);
  }

  // returns false is there is value
  bool is_none(void) {
    this->checked = true;
    return !(this->has_value);
  }

  // returns the value
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

  // returns refference to value if has_value is none throws runtime_error
  // if DEBUG is defined will throw invalid invalid_argument  if not checked
  T &operator*(void) {
#ifdef DEBUG
    if (checked == false)
      throw std::invalid_argument(
          "Tried to safe_unwrap without checking for none");

#endif
    if (has_value == true)
      return (value);
    else
      throw std::runtime_error("Derefferenced option that contained nothing");
  }

  // returns refference to value if has_value is none throws runtime_error
  // if DEBUG is defined will throw invalid invalid_argument  if not checked
  T *operator->(void) {
#ifdef DEBUG
    if (checked == false)
      throw std::invalid_argument(
          "Tried to safe_unwrap without checking for none");

#endif
    if (has_value == true)
      return (value);
    else
      throw std::runtime_error("Derreferenced option that contained nothing");
  }
  // returs refference to object doen't check if object exists if object doesn't
  // exist undefined behaviour
  // if DEBUG is defined will throw invalid invalid_argument  if not checked
  T &get_refference(void) {
#ifdef DEBUG
    if (checked == false)
      throw std::invalid_argument(
          "Tried to safe_unwrap without checking for none");

#endif
    return value;
  }
};
