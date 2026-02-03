#include "Listener.hpp"
#include "Networking.hpp"
#include "Stream.hpp"
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

Listener::Listener() : Networking(), pl_index(0), amount(nullptr) {}

Listener::Listener(const Listener &other) : Networking(), pl_index(other.pl_index), amount(other.amount) {
  if (amount) (*amount)++;
}

Listener::Listener(int fd) : Networking(), pl_index(fd) {
  if (initialized == false) {
    init();
    initialized = true;
  }
  amount = new size_t(1);
  pollarr[pl_index].fd = fd;
  pollarr[pl_index].events =
      POLLIN; // need to spesify what to listen to check if this is correct
  pollarr[pl_index].revents = 0;
}

Listener::~Listener() {
  if (amount) {
    (*amount)--;
    if (*amount == 0) {
      int fd = getFd();
      if (fd >= 0) {
        close(fd);
      }
      delete amount;
    }
  }
}

int Listener::getFd(void) const { return (pollarr[pl_index].fd); }
const Listener &Listener::operator=(const Listener &other) {
  this->pl_index = other.pl_index;
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
  int fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
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
    Result<Listener> rt("Failed to bind socket");
    return (rt);
  }
  if (listen(fd, SOMAXCONN) == -1) {
    Result<Listener> rt("Failed to listen on socket");
    return (rt);
  }
  Listener lis(fd);
  Result<Listener> rt = Result<Listener>(lis);
  return rt;
}
