#include "Socket.hpp"
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

Socket::Socket() {
  this->fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
  struct sockaddr data;
}

Socket::~Socket() { close(this->fd); }
Socket::Socket(const Socket &other) { *this = other; }

Socket &Socket::operator=(const Socket &other) {
  this->fd = other.fd;
  return (*this);
}

int Socket::getFd(void) const { return (this->fd); }

Result<Option<int>> Socket::accept() {
  int fd = ::accept(this->getFd(), NULL, NULL);
  if (fd == -1) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      Option<int> none(false);
      Result<Option<int>> rt(none);
      return (rt);
    } else {
      Result<Option<int>> rt(false, "Failed to accept client\n");
      return (rt);
    }
  }
  Option<int> some(fd);
  Result<Option<int>> rt(some);
  return (rt);
}

Result<Socket> Socket::bind() {
  Socket socket;
  if (socket.getFd() == -1) {
    Result<Socket> rt = Result<Socket>(false, "Failed to initialize_socket\n");
    return rt;
  }

  struct sockaddr_in server_adress;
  std::memset(&server_adress, 0, sizeof(sockaddr_in));
  server_adress.sin_family = AF_INET;
  server_adress.sin_port = htons(8080);
  server_adress.sin_addr.s_addr = INADDR_ANY;

  if (::bind(socket.getFd(), (struct sockaddr *)&server_adress,
             sizeof(sockaddr_in)) == -1) {
    Result<Socket> rt = Result<Socket>(false, "Failed to bind socket\n");
    return rt;
  }

  if (::listen(socket.getFd(), SOMAXCONN)) {
    Result<Socket> rt = Result<Socket>(false, "Failed to listen in socket\n");
    return rt;
  }

  Result<Socket> rt = Result<Socket>(socket);
  return (rt);
}

Result<Socket> Socket::connect() {
  Socket socket;
  if (socket.getFd() == -1) {
    Result<Socket> rt = Result<Socket>(false, "Failed to initialize_socket\n");
    return rt;
  }

  struct sockaddr_in server_adress;
  std::memset(&server_adress, 0, sizeof(sockaddr_in));
  server_adress.sin_family = AF_INET;
  server_adress.sin_port = htons(8080);
  // server_adress.sin_addr.s_addr = htonl(0);
  server_adress.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

  if (::connect(socket.getFd(), (struct sockaddr *)&server_adress,
                sizeof(sockaddr_in)) == -1) {
    Result<Socket> rt = Result<Socket>(false, "Failed to connect socket\n");
    return rt;
  }

  Result<Socket> rt = Result<Socket>(socket);
  return (rt);
}
