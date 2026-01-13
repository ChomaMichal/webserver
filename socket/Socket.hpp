#pragma once
#include "../utils/Option.hpp"
#include "../utils/Result.hpp"
#include "Requests.hpp"
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

class Socket {
private:
  int fd;

public:
  int getFd(void) const;
  Socket();
  Socket(const Socket &other);
  Result<Option<Request>> accept();
  ~Socket();
  Socket &operator=(const Socket &other);
  static Result<Socket> connect(void);
  static Result<Socket> bind(void);
};
