#include "Stream.hpp"
#include <cerrno>
#include <iostream>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

Stream::Stream() { std::cerr << "Don't use me like this Stream" << std::endl; }
Stream::Stream(struct pollfd &pl) : pl(pl) {}

Result<Option<Stream>> Stream::accept(Listener &lis) {
  errno = 0;
  int fd = ::accept(lis.getFd(), NULL, NULL);
  if (fd == -1) {
    Result<Option<Stream>> rt("Falied to accept check errno");
    return (rt);
  }
}

Stream stream();
Option<Stream> maybe(stream);
Result<Option<Stream>> rt(maybe);
return rt;
}

int Stream::getFd(void) const { return (this->fd); }
Option<std::string> Stream::read() {}
