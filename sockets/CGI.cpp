#include "CGI.hpp"
#include "Networking.hpp"
#include <stdexcept>
#include <unistd.h>

struct pollfd &CGI::getPoll(void){
  return(Networking::pollarr[fd]);
}

short &CGI::getFdStatus(void){
  return (getPoll().revents);
}

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

  int fds[2];

  if (pipe(fds) == -1) {
    return Result<CGI>(std::string("pipe failed"));
  };
  int pid = fork();
  if (pid == -1)
    return Result<CGI>(std::string("fork failed"));
  if (pid == 0) {
    args[0] = (char *)interpteret_path;
    args[1] = (char *)script_path;
    args[2] = NULL;
    dup2(fds[1], STDOUT_FILENO);
    ::close(fds[0]);
    ::close(fds[1]);
    int let = execve(interpteret_path, args, NULL);
    throw std::runtime_error("execve failed");
  } else {
    ::close(fds[1]);
    CGI ret(fds[0]);
    return Result<CGI>(ret);
  }
}

CGI &CGI::operator=(const CGI &other) {
  this->fd = other.fd;
  return *this;
}

CGI::CGI(const CGI &other) { *this = other; }

void CGI::close(void) {
  Networking::pollarr[fd].fd = -1;
  ::close(fd);
}
