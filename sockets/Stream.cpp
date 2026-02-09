#include "Stream.hpp"
#include "Listener.hpp"
#include "Networking.hpp"
#include <cerrno>
#include <cstring>
#include <iostream>
#include <new>
#include <stdio.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#ifdef NOALLOC
Stream prealloc_stream[MAX_STREAMS];
#endif

size_t Stream::fd_refcount[FD_MAX];

Stream::Stream()
    : Networking(), pl_index(0), buffer(new char[REQUEST_BODY_MAX]),
      pl(Networking::pollarr[0]) {
  Networking::pollarr[0].fd = -1;
}
Stream::Stream(struct pollfd &fd_ref)
    : Networking(), pl_index(fd_ref.fd), pl(fd_ref) {
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
  if (pl_index >= 0 && pl_index < FD_MAX) {
    fd_refcount[pl_index] = 1;
  }
}
Stream::Stream(const Stream &other)
    : Networking(), pl_index(other.pl_index), buffer(other.buffer),
      pl(Networking::pollarr[pl_index]) {
  if (pl_index >= 0 && pl_index < FD_MAX) {
    fd_refcount[pl_index]++;
  }
}

Stream::Stream(int fd) : Networking(), pl_index(fd), pl(pollarr[pl_index]) {
  buffer = new char[REQUEST_BODY_MAX];
  fd_refcount[fd] = 1;
  pollarr[pl_index].fd = fd;
  pollarr[pl_index].events = POLLIN | POLLOUT;
  pollarr[pl_index].revents = 0;
}

Stream::~Stream() {
  if (pl_index >= 0 && pl_index < FD_MAX) {
    fd_refcount[pl_index]--;
    if (fd_refcount[pl_index] == 0) {
      close(pl_index);
    }
  }
#ifndef NOALLOC
  delete[] buffer;
#endif
}

Stream &Stream::operator=(const Stream &other) {
  if (this != &other) {
    if (pl_index >= 0 && pl_index < FD_MAX) {
      fd_refcount[pl_index]--;
      if (fd_refcount[pl_index] == 0) {
        close(pl_index);
      }
    }
    buffer = other.buffer;
    pl_index = other.pl_index;
    if (pl_index >= 0 && pl_index < FD_MAX) {
      fd_refcount[pl_index]++;
    }
  }
  return *this;
}

short Stream::getFdStatus(void) {
  return (Networking::pollarr[pl_index].revents);
}
void Stream::printBuffer(void) const { std::cout << buffer << std::endl; }

Result<bool> Stream::read(void) {
  if (pollarr[pl_index].revents & (POLLIN | POLLHUP)) {
    int fd_to_read = pollarr[pl_index].fd;
    std::cout << "Attempting to read from fd=" << fd_to_read
              << " (pl_index=" << pl_index << ")" << std::endl;
    size_t rt = ::read(fd_to_read, buffer, REQUEST_BODY_MAX);
    if (rt == -1) {
      std::cerr << "Read error: " << strerror(errno) << std::endl;
      return (Result<bool>("Error on reading"));
    }
    std::cout << "Read " << rt << " bytes" << std::endl;
    buffer[rt] = 0;
    bool hehe = true;
    return (Result<bool>(hehe));
  } else if (pollarr[pl_index].revents & (POLLERR | POLLHUP)) {
    std::cerr << "Read error: " << strerror(errno) << std::endl;
    return (Result<bool>("Error on poll"));
  } else {
    bool hehe = false;
    return (Result<bool>(hehe));
  }
} // add errorhandeling for poll stuff

Result<Option<Stream>> Stream::accept(Listener &lis) {
  short events = lis.getFdStatus();
  if (events & (POLLERR | POLLHUP)) {
#ifdef DEBUG
    perror("error on sockethas failed");
#endif
    Result<Option<Stream>> rt("error on socket");
    return (rt);
  }
  if (events & POLLIN) {
    int fd = ::accept(lis.getFd(), NULL, NULL);
    if (fd == -1) {
#ifdef DEBUG
      perror("accppt has failed");
#endif
      Result<Option<Stream>> rt("error on accept");
      return rt;
    } else {
      lis.getPollarr()[fd].fd = fd;
      lis.getPollarr()[fd].events = POLLIN | POLLOUT;
      lis.getPollarr()[fd].revents = 0;
#ifdef NOALLOC
      Stream &stream = Networking::prealoc_stream[fd];
      stream.setPl(lis.getPollarr()[fd]);
      // Increment refcount to keep the fd from being closed
      if (fd >= 0 && fd < FD_MAX) {
        fd_refcount[fd]++;
      }
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
