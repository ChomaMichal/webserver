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
  NetworkIO(int fd) : fd(fd), buffer("") {}
  NetworkIO &operator=(NetworkIO &other);
  ~NetworkIO();
  void setFd(int fd) { this->fd = fd; }
  NetworkIO &operator>>(std::string &recieve);
  NetworkIO &operator<<(_finish &value);
  template <typename T> NetworkIO &operator<<(const T &value) {
    std::ostringstream os;
    os << value;
    buffer.append(os.str());
    return (*this);
  }
};

extern NetworkIO::_finish sendData;
