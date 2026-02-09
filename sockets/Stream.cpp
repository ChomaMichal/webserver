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

Stream::Stream()
    : Networking(), pl_index(0), buffer(new char[REQUEST_BODY_MAX]),
      pl(Networking::pollarr[0]) {
  Networking::pollarr[0].fd = -1;
}

Stream::Stream(const Stream &other)
    : Networking(), pl_index(other.pl_index), buffer(other.buffer),
      pl(Networking::pollarr[pl_index]) {}

Stream::~Stream() {}

Stream &Stream::operator=(const Stream &other) {
  this->pl = other.pl;
  this->prealoc_stream = other.prealoc_stream;
  this->pl_index = other.pl_index;
  return (*this);
}

short Stream::getFdStatus(void) {
  return (Networking::pollarr[pl_index].revents);
}
void Stream::printBuffer(void) const { std::cout << buffer << std::endl; }

Result<bool> Stream::read(void) {

  if (pollarr[pl_index].revents & (POLLIN | POLLHUP)) {

    size_t rt = ::read(getFd(), buffer, REQUEST_BODY_MAX);
    if (rt == -1) {
      std::cerr << "Read error: " << strerror(errno) << std::endl;
      return (Result<bool>("Error on reading"));
    }
    std::cout << "Read " << rt << " bytes" << std::endl;
    buffer[rt] = 0;
    bool hehe = true;
    return (Result<bool>(hehe));
  }

  else if (pollarr[pl_index].revents & (POLLERR | POLLHUP)) {
    std::cerr << "Read error: " << strerror(errno) << std::endl;
    return (Result<bool>("Error on poll"));
  } else {
    bool hehe = false;
    return (Result<bool>(hehe));
  }
} // add errorhandeling for poll stuff

char *Stream::getBuffer() { return (buffer); }

Result<Option<Stream>> Stream::accept(Listener &lis) {

  if (Networking::free_use.isFull() == true) {
    Option<Stream> none(false);
    Result<Option<Stream>> rt(none);
  }

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
    }

    else {
      size_t loc = free_use.pop();
      Stream stream = Networking::prealoc_stream[loc];
      stream.loc_of_alloc = loc;
      Networking::pollarr[fd].fd = fd;
      Networking::pollarr[fd].events = POLLIN | POLLHUP | POLLERR;
      Networking::pollarr[fd].revents = 0;
      stream.pl_index = fd;
      Option<Stream> some(stream);
      Result<Option<Stream>> rt(some);
      return (rt);
    }
  }
  Option<Stream> none(false);
  Result<Option<Stream>> rt(none);
  return (none);
}

void Stream::close(void) {
  ::close(pl.fd);
  pl.events = 0;
  pl.revents = 0;
  pl_index = 0;
  free_use.push(loc_of_alloc);
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
