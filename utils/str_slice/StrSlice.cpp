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

char &StrSlice::at(size_t pos) { return (allocation[pos]); }
