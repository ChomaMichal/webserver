#pragma once
#include <cstddef>
#include <iostream>
#include <sstream>
#include <unistd.h>

class NetworkIO {
private:
  int fd;
  std::string buffer;

public:
  struct _finish {};
  NetworkIO();
  NetworkIO(NetworkIO &other);
  NetworkIO &operator=(NetworkIO &other);
  ~NetworkIO();
  NetworkIO &operator>>(std::string &recieve);
  NetworkIO &operator<<(_finish &value);
  template <typename T> NetworkIO &operator<<(const T &value) {
    std::ostringstream os;
    buffer.append((os << value).str());
    return (*this);
  }
};
extern NetworkIO::_finish send;
