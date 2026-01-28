#pragma once
#include "../utils/option/Option.hpp"
#include "../utils/result/Result.hpp"
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>

#ifndef FD_MAX
#define FD_MAX 4096
#endif

class Listener {
public:
  Listener(const Listener &other);
  const Listener &operator=(const Listener &other);
  static Result<Listener> connect(int port);
  // calls poll to check status of all fds managed by this library
  static void update_fd_status(void);
  // returns the fd of the socket that listens for requests
  int getFd(void) const;
  short getRevent(int fd);
  // returns revents set by last call of poll
  short getFdStatus(void);
  ~Listener();
  struct pollfd *getPollarr(void);
  // never use outise of internal funcion
  Listener(int fd);
  Listener();

private:
  size_t *amount;
  struct pollfd &pl;
  static struct pollfd pollarr[FD_MAX];
  static bool initialized;
  // will initialize the pollfd arr to be usable
  // does it on the firste initialize
  static void init(void);
};
