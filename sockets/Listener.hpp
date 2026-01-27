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
  Listener();
  Listener(const Listener &other);
  Listener(int fd);
  const Listener &operator=(const Listener &other);
  static Result<Listener> connect(int port);
  static void init(void);
  int getFd(void) const;
  short getRevents(void);
  ~Listener();
  struct pollfd *getPollarr(void);

private:
  struct pollfd &pl;
  static struct pollfd pollarr[FD_MAX];
};
