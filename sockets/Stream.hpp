#pragma once
#include "../utils/option/Option.hpp"
#include "../utils/result/Result.hpp"
#include "../utils/str_slice/StrSlice.hpp"
#include "Listener.hpp"
#include "Networking.hpp"
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>

#ifndef REQUEST_BODY_MAX
#define REQUEST_BODY_MAX 1000000
#endif

class Stream : Networking {
private:
  static size_t fd_refcount[FD_MAX];

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
  Stream();
  Stream(struct pollfd &fd);
  Stream(int fd);

private:
  size_t pl_index;
  char *buffer;
  struct pollfd &pl;
};
