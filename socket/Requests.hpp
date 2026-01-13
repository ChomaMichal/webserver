#pragma once
#include "../utils/Result.hpp"
#include <poll.h>
#include <vector>

class Requests {
public:
  Requests(void);
  Requests(Requests &other);
  Requests &operator=(Requests &other);
  ~Requests();
  Result<bool> handle(void);
  void add(int fd);

private:
  std::vector<struct pollfd> fds;
};
