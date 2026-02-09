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
  Stream &operator=(const Stream &other);
  static Result<Option<Stream>> accept(Listener &lis);
  int getFd(void) const;
  void setPl(const struct pollfd &fd);
  short getFdStatus(void);
  void printBuffer(void) const;
  void close(void);

  Stream();

private:
  size_t pl_index;
  char *buffer;
  struct pollfd &pl;
  size_t loc_of_alloc;
};
