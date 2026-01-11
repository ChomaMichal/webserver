#pragma once

#include <stdexcept>
#include <string>

template <typename T>

class Result {
protected:
  T value;
  std::string error_message;
  bool correct;
  bool checked;

private:
  Result() {}

public:
  Result(Result &other) { *this = other; }
  Result &operator=(Result &other) {
    this->checked = other.checked;
    this->correct = other.correct;
    this->value = other.value;
    this->error_message = other.error_message;
    return *this;
  }
  Result(T &value) : value(value), correct(true), checked(false) {}
  Result(bool err, const std::string &error_message)
      : correct(false), checked(false), error_message(error_message) {}
  T safe_unwrap(void) {
    if (checked == false)
      throw std::invalid_argument(
          "Tried to safe_unwrap without checking for err");

    if (correct == false)
      throw std::runtime_error(error_message);
    else
      return value;
  }
  T unwrap(void) {
    if (checked == true && correct == true)
      return this->value;

    if (correct == false)
      throw std::runtime_error(error_message);
    else
      return value;
  }
  bool check(void) {
    this->checked = true;
    return (this->correct);
  }
  T unwrap_or(T &other) {
    if (checked == true && correct == true)
      return this->value;

    if (correct == false)
      return (other);
    else
      return value;
  }
  T expect(const std::string &error_mesage) {
    if (checked == true && correct == true)
      return this->value;

    if (correct == false)
      throw std::runtime_error(error_mesage);
    else
      return value;
  }
  ~Result() {}
};
