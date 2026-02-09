#pragma once
#include <stdexcept>
#include <string>
template <typename T>

class Result {
protected:
  T value;
  bool no_error;
  bool checked;
  std::string error_code;

public:
  Result(const std::string &error_code)
      : no_error(false), error_code(error_code), checked(false) {}
  Result() : no_error(false), checked(false) {}
  Result(T &value) : value(value), no_error(true), checked(false) {}
  Result(const Result &other) { *this = other; }
  Result &operator=(const Result &other) {
    this->no_error = other.no_error;
    this->value = other.value;
    this->error_code = other.error_code;
    this->checked = other.checked;
    return (*this);
  }

  // returns true is there is error
  bool is_error(void) {
    checked = true;
    return !(no_error);
  }

  // returns true is there is no_error
  bool is_corect(void) {
    checked = true;
    return (no_error);
  }

  std::string get_error(void) {
    if (no_error == false)
      return (error_code);
    return (std::string());
  }

  // returns the value
  T unwrap_or(T &other) {
    if (checked == true && no_error == true)
      return this->value;

    if (no_error == false)
      return (other);
    else
      return value;
  }
  // returns the value
  T unwrap() {

#ifdef DEBUG
    if (checked == false)
      throw std::invalid_argument(
          "Tried to safe_unwrap without checking for none");

#endif
    if (no_error == true)
      return this->value;
    else
      throw std::runtime_error("idk"); // change later
  }

  T expect(const std::string &error_mesage) {
    if (checked == true && no_error == true)
      return this->value;

    if (no_error == false)
      throw std::runtime_error(error_mesage);
    else
      return value;
  }

  // returns refference to value if has_value is none throws runtime_error
  // if DEBUG is defined will throw invalid invalid_argument  if not checked
  T &operator*(void) {
#ifdef DEBUG
    if (checked == false)
      throw std::invalid_argument(
          "Tried to safe_unwrap without checking for none");

#endif
    if (no_error == true)
      return (value);
    else
      throw std::runtime_error("Derefferenced result that contained nothing");
  }
  // returns refference to value if has_value is none throws runtime_error
  // if DEBUG is defined will throw invalid invalid_argument  if not checked
  T *operator->(void) {
#ifdef DEBUG
    if (checked == false)
      throw std::invalid_argument(
          "Tried to safe_unwrap without checking for none");

#endif
    if (no_error == true)
      return (value);
    else
      throw std::runtime_error("Derreferenced error that contained nothing");
  }
  // returs refference to object doen't check if object exists if object doesn't
  // exist undefined behaviour
  // if DEBUG is defined will throw invalid invalid_argument  if not checked
  T &get_refference(void) {
#ifdef DEBUG
    if (checked == false)
      throw std::invalid_argument(
          "Tried to get_refference on refference without checking for none");

#endif
    return value;
  }

  // returs pointer to object doen't check if object exists if object doesn't
  // exist undefined behaviour
  // if DEBUG is defined will throw invalid invalid_argument  if not checked
  T *get_pointer(void) {
#ifdef DEBUG
    if (checked == false)
      throw std::invalid_argument(
          "Tried to get_pointer on result without checking for none");

#endif
    return &value;
  }
};
