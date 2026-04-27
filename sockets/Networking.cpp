#include "Networking.hpp"
#include "Stream.hpp"

#include <netinet/in.h>
#include <sys/socket.h>

struct pollfd Networking::pollarr[FD_MAX];
bool Networking::initialized = false;
size_t Networking::_send_buffer_len = 0;
char Networking::_send_buffer[MAX_SEND_BUFFER];

#ifdef NOALLOC
char **Networking::prealoc_buff = nullptr;
std::stack<int> Networking::free_use;
#endif

Networking::Networking() {}

Networking::Networking(const Networking &) {}

Networking &Networking::operator=(const Networking &) { return *this; }

void Networking::init(void) {
  _send_buffer_len = 0;
  for (int i = 0; i < FD_MAX; i++) {
    pollarr[i].fd = -1;
    pollarr[i].events = 0;
    pollarr[i].revents = 0;
  }
#ifdef NOALLOC
  prealoc_buff = new char *[MAX_STREAMS];
  for (size_t i = 0; i < MAX_STREAMS; i++){
    prealoc_buff[i] = new char[MAX_SEND_BUFFER];
  }

  
  for (size_t i = 0; i < MAX_STREAMS; i++) {
    free_use.push(i);
  }
#endif
}
int Networking::update_fd_status(void) { return poll(pollarr, FD_MAX, 0); }

Networking::~Networking() {}

char **Networking::getPrealocBuff(void) { return (prealoc_buff); }
