#include "Listener.hpp"
#include "Networking.hpp"
#include "Stream.hpp"
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

Listener::Listener() : Networking(), fd(0) {}

Listener::Listener(const Listener &other) : Networking(), fd(other.fd) {}

Listener::Listener(int fd) : Networking(), fd(fd) {
  if (initialized == false) {
    init();
    initialized = true;
  }
  pollarr[fd].fd = fd;
  pollarr[fd].events = POLLIN;
  pollarr[fd].revents = 0;
}

Listener::~Listener() {}

int Listener::getFd(void) const { return (pollarr[fd].fd); }
const Listener &Listener::operator=(const Listener &other) {
  fd = other.fd;
  return *this;
}

short Listener::getFdStatus(void) { return pollarr[fd].revents; }

Result<Option<Stream>> Listener::accept() { return (Stream::accept(*this)); }

void Listener::init(void) { Networking::init(); }

struct pollfd *Listener::getPollarr(void) { return Networking::pollarr; }

Result<Listener> Listener::connect(int port) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    Result<Listener> rt("Failed to create socket");
    return (rt);
  }

  int opt = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
    ::close(fd);
    Result<Listener> rt("Failed to set SO_REUSEADDR");
    return (rt);
  }
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1) {
    ::close(fd);
    Result<Listener> rt("Failed to set SO_REUSEPORT");
    return (rt);
  }

  struct sockaddr_in server_adress;
  std::memset(&server_adress, 0, sizeof(sockaddr_in));
  server_adress.sin_family = AF_INET;
  server_adress.sin_port = htons(port);
  server_adress.sin_addr.s_addr = INADDR_ANY;
  if (bind(fd, (struct sockaddr *)&server_adress, sizeof(sockaddr_in)) == -1) {
    ::close(fd);
    Result<Listener> rt("Failed to bind socket");
    return (rt);
  }
  if (listen(fd, SOMAXCONN) == -1) {
    ::close(fd);
    Result<Listener> rt("Failed to listen on socket");
    return (rt);
  }

  Listener lis(fd);
  Result<Listener> rt = Result<Listener>(lis);
  return rt;
}

void Listener::close(void) {
  ::close(Networking::pollarr[fd].fd);
  Networking::pollarr[fd].fd = -1;
}
