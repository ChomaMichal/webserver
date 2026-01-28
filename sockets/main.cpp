#include "Listener.hpp"
int main() {
  Listener::connect(8080);
  for (int i = 0; i < 1000000; i++)
    Listener::update_fd_status();
}
