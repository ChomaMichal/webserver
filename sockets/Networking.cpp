#include "Networking.hpp"
#include "Stream.hpp"

#include <netinet/in.h>
#include <sys/socket.h>

struct pollfd Networking::pollarr[FD_MAX];
bool Networking::initialized = false;

#ifdef NOALLOC
Stream *Networking::prealoc_stream = nullptr;
#endif

Networking::Networking() {}

Networking::Networking(const Networking &) {}

Networking &Networking::operator=(const Networking &) {
  return *this;
}

void Networking::init(void) {
  for (int i = 0; i < FD_MAX; i++) {
    pollarr[i].fd = -1;
    pollarr[i].events = 0;
    pollarr[i].revents = 0;
  }
#ifdef NOALLOC
  if (!prealoc_stream) {
    prealoc_stream = new Stream[1024];
  }
#endif
}
int Networking::update_fd_status(void) {
  return (
      poll(pollarr, 4096, 0)); // test with -1 it blocks untill event happends
}

Networking::~Networking() {
  // Don't delete prealoc_stream here - it's a static resource
  // Only delete when the program exits
}
