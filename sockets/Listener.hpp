#pragma once
#include "../utils/option/Option.hpp"
#include "../utils/result/Result.hpp"
#include "Networking.hpp"
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

class Listener : Networking {
private:
  static size_t fd_refcount[FD_MAX];
  
public:
  Listener(const Listener &other);
  const Listener &operator=(const Listener &other);
  static Result<Listener> connect(int port);
  int getFd(void) const;
  short getRevent(int fd);
  short getFdStatus(void);
  ~Listener();
  struct pollfd *getPollarr(void);
  Listener(int fd);
  Listener();
  Result<Option<Stream>> accept(void);
  
private:
  size_t pl_index;
  static void init(void);
};
