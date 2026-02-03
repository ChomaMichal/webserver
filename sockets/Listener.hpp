#pragma once
#include "../utils/option/Option.hpp"
#include "../utils/result/Result.hpp"
#include "Networking.hpp"
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>

class Listener : Networking {
public:
  Listener(const Listener &other);
  const Listener &operator=(const Listener &other);
  static Result<Listener> connect(int port);
  // calls poll to check status of all fds managed by this library
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
  Result<Option<Stream>> accept(void);

private:
  size_t *amount;
  size_t pl_index;
  static void init(void);
};
