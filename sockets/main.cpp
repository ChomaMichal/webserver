#include "Listener.hpp"
#include "Networking.hpp"
int main() {
  Listener::connect(8080);
  for (int i = 0; i < 1000000; i++)
    Networking::update_fd_status();
}
