#include "Networking.hpp"
#include "Stream.hpp"

#include <netinet/in.h>
#include <sys/socket.h>

struct pollfd Networking::pollarr[FD_MAX];
bool Networking::initialized = false;

#ifdef NOALLOC
Stream *Networking::prealoc_stream = nullptr;
Stack<int> Networking::free_use;
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
  prealoc_stream = new Stream[MAX_STREAMS];
  for (int i = 0; i < MAX_STREAMS; i++) {
    prealoc_stream[i].setPl(pollarr[i]);
  }

  Stack<int> stack(MAX_STREAMS + 1);
  for (size_t i = 0; i < MAX_STREAMS; i++) {
    stack.push(i);
  }
  free_use = stack;
#endif
}
int Networking::update_fd_status(void) { return poll(pollarr, FD_MAX, 0); }

Networking::~Networking() {}

Stream *Networking::getPrealocStream(void) { return (prealoc_stream); }
