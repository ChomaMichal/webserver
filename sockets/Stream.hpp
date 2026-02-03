#pragma once
#include "../utils/option/Option.hpp"
#include "../utils/result/Result.hpp"
#include "../utils/str_slice/StrSlice.hpp"
#include "Listener.hpp"
#include "Networking.hpp"
#include <sys/poll.h>
#include <sys/socket.h>

#ifndef REQUEST_BODY_MAX
#define REQUEST_BODY_MAX 1000000
#endif

// idea behind this only way to construct this class is by the funcioon accept
// that will be called by listener
// this class will allocate on initialization, will never allocate during
// oprations

class Stream : Networking {
public:
  Stream(const Stream &other);
  ~Stream();
  // returns true if something was read
  Result<bool> read(void);
  Stream &operator=(const Stream &other);
  static Result<Option<Stream>> accept(Listener &lis);
  int getFd(void) const;
  void setPl(const struct pollfd &fd);
  void printBuffer(void) const;
  Stream();

private:
  char *buffer;
  StrSlice header;
  StrSlice body;
  Stream(struct pollfd &fd);
  Stream(int fd);
  size_t pl_index;
};
