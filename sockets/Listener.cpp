#include "Listener.hpp"
#include "Networking.hpp"
#include "Stream.hpp"
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

size_t Listener::fd_refcount[FD_MAX];

Listener::Listener() : Networking(), pl_index(0) {}

Listener::Listener(const Listener &other) : Networking(), pl_index(other.pl_index) {
  if (pl_index >= 0) {
    fd_refcount[pl_index]++;
  }
}

Listener::Listener(int fd) : Networking(), pl_index(fd) {
  if (initialized == false) {
    init();
    initialized = true;
  }
  fd_refcount[fd] = 1;
  pollarr[pl_index].fd = fd;
  pollarr[pl_index].events = POLLIN;
  pollarr[pl_index].revents = 0;
}

Listener::~Listener() {
  if (pl_index >= 0 && pl_index < FD_MAX) {
    fd_refcount[pl_index]--;
    if (fd_refcount[pl_index] == 0) {
      close(pl_index);
    }
  }
}

int Listener::getFd(void) const { return (pollarr[pl_index].fd); }
const Listener &Listener::operator=(const Listener &other) {
  if (this != &other) {
    if (pl_index >= 0 && pl_index < FD_MAX) {
      fd_refcount[pl_index]--;
      if (fd_refcount[pl_index] == 0) {
        close(pl_index);
      }
    }
    this->pl_index = other.pl_index;
    if (pl_index >= 0 && pl_index < FD_MAX) {
      fd_refcount[pl_index]++;
    }
  }
  return *this;
}

short Listener::getFdStatus(void) { return pollarr[pl_index].revents; }

Result<Option<Stream>> Listener::accept() { return (Stream::accept(*this)); }

void Listener::init(void) {
  Networking::init();
}

struct pollfd *Listener::getPollarr(void) {
  return Networking::pollarr;
}

Result<Listener> Listener::connect(int port) {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  if (fd == -1) {
    Result<Listener> rt("Failed to create socket");
    return (rt);
  }
  
  struct sockaddr_in server_adress;
  std::memset(&server_adress, 0, sizeof(sockaddr_in));
  server_adress.sin_family = AF_INET;
  server_adress.sin_port = htons(port);
  server_adress.sin_addr.s_addr = INADDR_ANY;
  if (bind(fd, (struct sockaddr *)&server_adress, sizeof(sockaddr_in)) == -1) {
    close(fd);
    Result<Listener> rt("Failed to bind socket");
    return (rt);
  }
  if (listen(fd, SOMAXCONN) == -1) {
    close(fd);
    Result<Listener> rt("Failed to listen on socket");
    return (rt);
  }
  
  Listener lis(fd);
  Result<Listener> rt = Result<Listener>(lis);
  return rt;
}
