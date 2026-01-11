#include "Socket.hpp"
#include <vector>

void server(void) {
  auto maybe = Socket::bind();
  Socket soc;
  std::vector<int> clients;

  if (maybe.check()) {
    soc = maybe.safe_unwrap();
  }

  while (true) {
    auto err = soc.accept();
    if (err.check() == false) {
      std::cerr << "error\n";
      return;
    }
    auto maybe = err.safe_unwrap();
    if (maybe.check()) {
      clients.push_back(maybe.safe_unwrap());
    }
  }
}

int main() {
  int pid = fork();
  if (pid != 0)
    server();
}
