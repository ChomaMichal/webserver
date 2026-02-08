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

Networking &Networking::operator=(const Networking &) { return *this; }

void Networking::init(void) {
  for (int i = 0; i < FD_MAX; i++) {
    pollarr[i].fd = -1;
    pollarr[i].events = 0;
    pollarr[i].revents = 0;
  }
#ifdef NOALLOC
  if (!prealoc_stream) {
    prealoc_stream = new Stream[1024];
    for (int i = 0; i < MAX_STREAMS; i++) {
      prealoc_stream[i].setPl(pollarr[i]);
    }
  }
#endif
}
int Networking::update_fd_status(void) { return poll(pollarr, FD_MAX, 0); }

Networking::~Networking() {}

Stream *Networking::getPrealocStream(void) { return (prealoc_stream); }
