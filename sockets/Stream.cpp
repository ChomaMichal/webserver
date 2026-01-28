#include "Stream.hpp"
#include "Listener.hpp"
#include <errno.h>
#include <stdio.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

Stream::Stream(struct pollfd &pl) : pl(pl) {}

Result<Option<Stream>> Stream::accept(Listener &lis) {
  short events = lis.getFdStatus();
  if (events == POLLERR || events == POLLHUP) {
#ifdef DEBUG
    perror("error on sockethas failed");
#endif
    Result<Option<Stream>> rt("error on socket");
    return (rt);
  }
  if (events == POLLIN) {
    int fd = ::accept(lis.getFd(), NULL, NULL);
    if (fd == -1) {
#ifdef DEBUG
      perror("accppt has failed");
#endif
      Result<Option<Stream>> rt("error on accept");
      return rt;
    } else {
      lis.getPollarr()[fd].fd = fd;
      Stream stream(lis.getPollarr()[fd]);
      Option<Stream> some(stream);
      Result<Option<Stream>> rt(some);
      return (rt);
    }
  }
  Option<Stream> none(false);
  Result<Option<Stream>> rt(none);
  return (none);
}

int Stream::getFd(void) const { return (this->pl.fd); }
Option<std::string> Stream::read() {}
