#pragma once
#include "../utils/result/Result.hpp"
#include "Networking.hpp"

class CGI : Networking {
public:
  void close(void);
  const int &getFd(void);
  CGI(const CGI &other);
  struct pollfd &getPoll();
  short &getFdStatus();
  static Result<CGI> run_script(const char *script_path,
                                const char *interpteret_path);
  CGI &operator=(const CGI &other);

private:
  CGI();
  CGI(int fd);
  int fd;
};
