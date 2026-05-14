#pragma once
#include "../utils/option/Option.hpp"
#include "../utils/result/Result.hpp"
#include "Listener.hpp"
#include "Networking.hpp"
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#ifndef REQUEST_BODY_MAX
#define REQUEST_BODY_MAX 1000000
#endif

class Stream : Networking {

public:
  Stream(const Stream &other);
  ~Stream();
  Result<bool> read(void);
  Result<bool> write(void);
  Stream &operator=(const Stream &other);
  static Result<Option<Stream>> accept(Listener &lis);
  int getFd(void) const;
  short getFdStatus(void);
  struct pollfd &getPoll(void);
  void printBuffer(void) const;
  char *getBuffer(void);
  void close(void);
  Stream();

private:
  Stream(int fd, size_t loc);
  size_t pl_index;
  int fd;
};
