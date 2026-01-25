#pragma once
#include <cstddef>
#include <stdexcept>
#include <string>
// This contains part of the string and its length, It doesn;t realocate
class StrSlice {
public:
  StrSlice();
  ~StrSlice();
  StrSlice(const StrSlice &other);
  StrSlice &operator=(const StrSlice &other);
  // pass the string how many characters to ignore and the len of the slice
  StrSlice(std::string &string, size_t to_ignore, size_t len);
  // pass the string and len of the substring
  StrSlice(std::string &string, size_t len);
  // pass the string and len of the substring
  StrSlice(char *string, size_t);
  // access pos bit, if out of bounds throws exception
  char &operator[](size_t pos);
  // access pos bit, if out of bounds undefined behaviour
  char &at(size_t pos);

private:
  char *allocation;
  size_t len;
};
