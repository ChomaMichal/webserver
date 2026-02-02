#include "Stream.hpp"
#include "Listener.hpp"
#include "Networking.hpp"
#include <cstring>
#include <errno.h>
#include <new>
#include <stdio.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#ifdef NOALLOC
Stream prealloc_stream[MAX_STREAMS];
#endif

Stream::Stream() : pl(pollarr[0]), request(new char[REQUEST_BODY_MAX]) {}
Stream::Stream(struct pollfd &pl) : pl(pl) {}

Stream::Stream(int fd) : pl(pollarr[fd]) {
  Stream::request = new char[REQUEST_BODY_MAX];
  pl.fd = fd;
  pl.events = POLLIN | POLLOUT;
  pl.revents = 0;
}

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
#ifdef NOALLOC
      Stram
#else
      Stream stream(lis.getPollarr()[fd]);
#endif

          Option<Stream>
              some(stream);
      Result<Option<Stream>> rt(some);
      return (rt);
    }
  }
  Option<Stream> none(false);
  Result<Option<Stream>> rt(none);
  return (none);
}

int Stream::getFd(void) const { return (this->pl.fd); }
