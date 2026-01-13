#include "NetworkIO.hpp"
#include <cstring>

NetworkIO::_finish sendData;

NetworkIO::NetworkIO() : fd(-1) {}

NetworkIO::~NetworkIO() {}

NetworkIO::NetworkIO(NetworkIO &other) { *this = other; }

NetworkIO &NetworkIO::operator=(NetworkIO &other) {
  this->fd = other.fd;
  this->buffer = other.buffer;
  return (*this);
}

NetworkIO &NetworkIO::operator<<(_finish &send) {
  write(this->fd, this->buffer.c_str(), this->buffer.size());
  return (*this);
}

NetworkIO &NetworkIO::operator>>(std::string &recieve) {
  char buff[1024];
  size_t rt = 0;
  std::memset(buff, 0, 1024);
  while (true) {
    size_t rt = read(this->fd, buff, 1024);
    if (rt == -1) {
      std::cerr << "failed to read\n";
      return (*this); // errorhandle later
    }
    if (rt == 0)
      break;
    recieve.append(buff);
  }
  return (*this);
}
