#include "Requests.hpp"

Requests::Requests(void) {};
Requests::Requests(Requests &other) : fds(other.fds) {}
Requests &Requests::operator=(Requests &other) {
  this->fds = other.fds;
  return *this;
}
Requests::~Requests() {}
void Requests::add(int fd) {
  struct pollfd tmp;
  tmp.fd = fd;
  tmp.events = 0;
  fds.push_back(tmp);
}

Result<bool> Requests::handle(void) {
  if (poll(this->fds.data(), fds.size(), 0) == 0) {
    // nothing to do
    Result<bool> rt(true);
    return (rt);
  }

  auto iter = fds.begin();
  while (iter != fds.end()) {
    switch (iter->revents) {
    case POLLIN:
      // data to read
      break;
    case POLLOUT:
      // can write
      break;
    case POLLRDHUP:
      // closed connection
      break;
    case POLLERR:
      // error condition
      break;
    case POLLHUP:
      // hang up reading will read null bite because connection is closed
      break;
    case POLLNVAL:
      // invalid fd
      break;
    }
  }

  return (false);
}
