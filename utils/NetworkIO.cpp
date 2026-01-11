#include "NetworkIO.hpp"

NetworkIO::_finish send;

NetworkIO::NetworkIO() : fd(-1) {}

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
  do {
    size_t rt = read(this->fd, buff, 1024);
    if (rt == -1) {
      std::cerr << "failed to read\n";
      return (*this); // errorhandle later
    }
    recieve.append(buff);
  } while (rt != 0);
  return (*this);
}
