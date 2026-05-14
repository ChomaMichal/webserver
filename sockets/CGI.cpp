#include "CGI.hpp"
#include <unistd.h>

CGI::CGI(int fd) : fd(fd) {
  pollarr[fd].fd = fd;
  pollarr[fd].events = 0;
  pollarr[fd].revents = 0;
}

CGI::CGI() {}

const int &CGI::getFd(void) { return fd; }

Result<CGI> CGI::run_script(const char *script_path,
                            const char *interpteret_path) {
  char *args[3];

  args[0] = (char *)interpteret_path;
  args[1] = (char *)script_path;
  args[2] = NULL;
  int let = execve(interpteret_path, args, NULL);
  if (let == -1) {
    return Result<CGI>(std::string("execve failed"));
  }
  CGI ret(let);
  return Result<CGI>(ret);
}

CGI &CGI::operator=(const CGI &other) {
  this->fd = other.fd;
  return *this;
}

CGI::CGI(const CGI &other) { *this = other; }
