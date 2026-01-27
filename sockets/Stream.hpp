#pragma once
#include "../utils/option/Option.hpp"
#include "Listener.hpp"
#include <sys/poll.h>
#include <sys/socket.h>
#include <vector>

class Stream {
public:
  Stream(const Stream &other);
  Stream(struct pollfd &fd);
  Stream(int fd);
  ~Stream();
  Option<std::string> read(void);
  Stream &operator=(const Stream &other);
  static Result<Option<Stream>> accept(Listener &lis);
  int getFd(void) const;

private:
  Stream();
  struct pollfd &pl;
};
