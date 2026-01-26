#pragma once
#include "../utils/option/Option.hpp"
#include "../utils/result/Result.hpp"
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>

class Listener {
public:
  Listener();
  Listener(const Listener &other);
  Listener(int fd);
  const Listener &operator=(const Listener &other);
  static Result<Listener> connect(int port);
  int getFd(void) const;
  ~Listener();

private:
  int fd;
  int *amount;
};
