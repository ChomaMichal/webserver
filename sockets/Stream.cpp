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

Stream::Stream() : Networking(), pl_index(0), buffer(new char[REQUEST_BODY_MAX]) {}
Stream::Stream(struct pollfd &fd_ref) : Networking(), pl_index(0) {
#ifndef NOALLOC
  buffer = new char[REQUEST_BODY_MAX];
#endif
  // Find index of this pollfd in pollarr
  for (int i = 0; i < FD_MAX; i++) {
    if (&pollarr[i] == &fd_ref) {
      pl_index = i;
      break;
    }
  }
}
Stream::Stream(const Stream &other) : Networking(), pl_index(other.pl_index), buffer(other.buffer) {}

Stream::Stream(int fd) : Networking(), pl_index(fd) {
  Stream::buffer = new char[REQUEST_BODY_MAX];
  pollarr[pl_index].fd = fd;
  pollarr[pl_index].events = POLLIN | POLLOUT;
  pollarr[pl_index].revents = 0;
}

Stream::~Stream() {
#ifndef NOALLOC
  delete[] buffer;
#endif
}

Stream &Stream::operator=(const Stream &other) {
  if (this != &other) {
    buffer = other.buffer;
    pl_index = other.pl_index;
  }
  return *this;
}

void Stream::printBuffer(void) const { std::cout << buffer; }

Result<bool> Stream::read(void) {
  if (pollarr[pl_index].revents == POLLIN) {
    size_t rt = ::read(pollarr[pl_index].fd, buffer, REQUEST_BODY_MAX);
    if (rt == -1) {
      return (Result<bool>("Error on reading"));
    }
    buffer[rt] = 0;
    bool hehe = true;
    return (Result<bool>(hehe));
  } else {
    bool hehe = false;
    return (Result<bool>(hehe));
  }
} // add errorhandeling for poll stuff

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
      Stream &stream = Networking::prealoc_stream[fd];
      stream.setPl(lis.getPollarr()[fd]);
#else
      Stream stream(lis.getPollarr()[fd]);
#endif

      Option<Stream> some(stream);
      Result<Option<Stream>> rt(some);
      return (rt);
    }
  }
  Option<Stream> none(false);
  Result<Option<Stream>> rt(none);
  return (none);
}

int Stream::getFd(void) const { return (pollarr[pl_index].fd); }

void Stream::setPl(const struct pollfd &fd) {
  for (int i = 0; i < FD_MAX; i++) {
    if (&pollarr[i] == &fd) {
      pl_index = i;
      break;
    }
  }
}