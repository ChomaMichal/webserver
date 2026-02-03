#include "Networking.hpp"
#include "Stream.hpp"

#include <netinet/in.h>
#include <sys/socket.h>

struct pollfd Networking::pollarr[FD_MAX];
bool Networking::initialized;

void Networking::init(void) {
  for (int i = 0; i < FD_MAX; i++) {
    pollarr[i].fd = -1;
    pollarr[i].events = 0;
    pollarr[i].revents = 0;
  }
#ifdef NOALLOC
  prealoc_stream = new Stream[1024];
#endif
}
int Networking::update_fd_status(void) {
  return (
      poll(pollarr, 4096, 0)); // test with -1 it blocks untill event happends
}

Networking::~Networking() {
#ifdef NOALLOC
  delete[] prealoc_stream;
#endif
}
