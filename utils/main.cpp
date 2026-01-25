#include "option/Option.hpp"
#include <fcntl.h>
#include <iostream>

Option<int> ft_open(const std::string &filename) {
  int fd = open(filename.c_str(), O_RDWR);

  if (fd == -1) {
    Option<int> rt(false);
    return (rt);
  }
  Option<int> rt(fd);
  return rt;
}

int main() {
  auto maybe = ft_open("main.cpp");
  if (maybe.is_none()) {
    std::cerr << "failed to open" << std::endl;
    return 1;
  }
  int fd = *maybe;
  std::cout << fd;
}
