#include "StrSlice.hpp"

StrSlice::StrSlice() : allocation(NULL), len(0) {}
StrSlice::StrSlice(const StrSlice &other) { *this = other; }
StrSlice &StrSlice::operator=(const StrSlice &other) {
  this->allocation = other.allocation;
  this->len = other.len;
  return (*this);
}
StrSlice::~StrSlice() {}
StrSlice::StrSlice(std::string &string, size_t begining, size_t len)
    : allocation(string.data() + begining), len(len) {}

StrSlice::StrSlice(std::string &string, size_t len)
    : allocation(string.data()), len(len) {}

StrSlice::StrSlice(char *string, size_t len) : allocation(string), len(len) {}

char &StrSlice::operator[](size_t pos) {
  if (pos >= len) {
    throw std::out_of_range("Tried to access out of range of the string slice");
  }
  return (allocation[pos]);
}

const char &StrSlice::operator[](size_t pos) const {
  if (pos >= len) {
    throw std::out_of_range("Tried to access out of range of the string slice");
  }
  return (allocation[pos]);
}

char &StrSlice::at(size_t pos) { return (allocation[pos]); }
const char &StrSlice::at(size_t pos) const { return (allocation[pos]); }

size_t StrSlice::getLen(void) const { return (len); }

std::ostream &operator<<(std::ostream &os, const StrSlice &str) {
  for (size_t i = 0; i < str.getLen(); i++) {
    os << str.at(i);
  }
  return (os);
}
