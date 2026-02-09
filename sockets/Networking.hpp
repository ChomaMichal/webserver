#pragma once
#include "../utils/stack/Stack.hpp"
#include <sys/poll.h>

#define NOALLOC

#ifndef FD_MAX
#define FD_MAX 50
#endif

#ifdef NOALLOC
#ifndef MAX_STREAMS
#define MAX_STREAMS FD_MAX
#endif
#endif

#define DEBUG

class Stream;
// this the parent class;
// this class stores fds that are used in this library;
//  funcion init will initialize pollarr which is for checkoing availoibility of
//  fds, FD_MAX is set to 4096 which is system wide max
//  each subclass should have ists poll struct sotred in this pollarr
//  it should only use fds after checking status
class Networking {
protected:
  static struct pollfd pollarr[FD_MAX];
  static bool initialized;
  Networking();
  virtual ~Networking();
  Networking(const Networking &);
  virtual Networking &operator=(const Networking &);

#ifdef NOALLOC
  static Stream *prealoc_stream;
  static Stack<int> free_use;
#endif

public:
  // reutrnsn the number of available fds
  static int update_fd_status(void);
  // will initialize the pollfd arr to be usable
  static void init(void);
#ifdef NOALLOC
  static Stream *getPrealocStream(void);
#endif
public:
};
