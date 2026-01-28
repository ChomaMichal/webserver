#include "Listener.hpp"
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

struct pollfd Listener::pollarr[FD_MAX];
bool Listener::initialized;

Listener::Listener() : pl(this->pollarr[0]) {
  std::cerr << "Fuck of don't use me like this" << std::endl;
}

Listener::Listener(const Listener &other) : pl(other.pl), amount(other.amount) {
  (*amount)++;
}

Listener::Listener(int fd) : pl(this->pollarr[fd]) {
  if (initialized == false) {
    init();
    initialized = true;
  }
  amount = new (size_t);
  *amount = 1;
  pl.fd = fd;
  pl.events =
      POLLIN; // need to spesify what to listen to check if this is correct
}

Listener::~Listener() {
  amount--;
  if (amount == 0) {
    close(this->getFd());
    delete amount;
  }
}

int Listener::getFd(void) const { return (pl.fd); }
const Listener &Listener::operator=(const Listener &other) {
  this->pl = other.pl;
  return *this;
}

short Listener::getFdStatus(void) { return pl.revents; }

void Listener::init(void) {
  for (int i = 0; i < FD_MAX; i++) {
    pollarr[i].fd = -1;
    pollarr[i].events = 0;
    pollarr[i].revents = 0;
  }
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
  if (bind(fd, (struct sockaddr *)&server_adress, sizeof(sockaddr_in) == -1)) {
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

void Listener::update_fd_status(void) {
  poll(Listener::pollarr, 4096,
       0); // test with -1 it blocks untill event happends
}
