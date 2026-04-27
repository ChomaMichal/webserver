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

Stream::~Stream() {}

Stream::Stream()
    : Networking(), pl_index(0), buffer(new char[REQUEST_BODY_MAX]),
      pl(Networking::pollarr[0]) {
  Networking::pollarr[0].fd = -1;
}

Stream::Stream(const Stream &other)
    : Networking(), pl_index(other.pl_index), buffer(other.buffer),
      pl(Networking::pollarr[pl_index]) {}

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

// rewrite to Result<Option<Bool>>
Result<bool> Stream::read(void) {

  std::cout << "in read in stream :: 44" << std::endl;
  if (pollarr[pl_index].revents & (POLLIN | POLLHUP)) {
    std::cout << "after if in read in stream :: 46" << std::endl;
    size_t rt = ::read(getFd(), buffer, REQUEST_BODY_MAX);
    if (rt == -1) {
      std::cerr << "Read error: " << strerror(errno) << std::endl;
      return (Result<bool>("Error on reading"));
    }
    std::cout << "Read " << rt << " bytes" << std::endl;
    buffer[rt] = 0;
    bool hehe = true;
    pollarr[pl_index].revents = 0;
    std::cout << "after read in stream :: 55" << std::endl;
    return (Result<bool>(hehe));
  }

  else if (pollarr[pl_index].revents & (POLLERR | POLLHUP)) {
    std::cerr << "Read error: " << strerror(errno) << std::endl;
    return (Result<bool>("Error on poll"));
  } else {
    // static int tries = -1;
    // if (tries == -1) {
    //   tries = 5;
    // }
    // tries--;
    // if (tries == 0)
    //  return Result<bool>("maxed out");
    // std::cout << "here2\n";
    bool hehe = false;
    return (Result<bool>(hehe));
  }
}

// rewrite to Result<Option<Bool>>
Result<bool> Stream::write(void) {

  if (pollarr[pl_index].revents & (POLLOUT | POLLHUP)) {

    size_t rt = ::write(getFd(), _send_buffer, _send_buffer_len);
    if (rt == -1) {
      std::cerr << "Read write: " << strerror(errno) << std::endl;
      return (Result<bool>("Error on reading"));
    }
    bool hehe = true;
    pollarr[pl_index].revents = 0;
    return (Result<bool>(hehe));
  }

  else if (pollarr[pl_index].revents & (POLLERR | POLLHUP)) {
    std::cerr << "Write error: " << strerror(errno) << std::endl;
    return (Result<bool>("Error on poll"));
  } else {
    bool hehe = false;
    return (Result<bool>(hehe));
  }
}

Result<Option<Stream>> Stream::accept(Listener &lis) {

  if (Networking::free_use.isFull() == true) {
    Option<Stream> none(false);
    Result<Option<Stream>> rt(none);
  }

  short events = lis.getFdStatus();

  if (events == 0) {
    Option<Stream> none(false);
    Result<Option<Stream>> rt(none);
    return (none);
  }

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
      Networking::pollarr[fd].events = POLLIN | POLLOUT | POLLHUP | POLLERR;
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

char *Stream::getBuffer(void) { return (buffer); }

void Stream::setPl(const struct pollfd &fd) {
  for (int i = 0; i < FD_MAX; i++) {
    if (&pollarr[i] == &fd) {
      pl_index = i;
      break;
    }
  }
}
