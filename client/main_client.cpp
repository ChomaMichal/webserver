#include "../sockets/Listener.hpp"
#include "../sockets/Networking.hpp"
#include "Client.hpp"
#include "Request.hpp"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <list>
#include <netinet/in.h>
#include <new>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

const int PORT = 2222;
const int BUFFER_SIZE = 4096;

int main() {
  Networking::init();
  auto lis = Listener::connect(PORT);
  if (lis.is_error()) {
    std::cerr << lis.get_error() << std::endl;
    return (1);
  }
  std::list<Client> arr;
  std::cout << "Send an HTTP request using: curl http://127.0.0.1:" << PORT
            << std::endl;
  while (1) {
    if (Networking::update_fd_status() == 0) {
      sleep(1);
      continue;
    }
    if ((*lis).getFdStatus() == POLLIN) {
      auto res = Stream::accept(*lis);
      if (res.is_error()) {
        std::cerr << res.get_error() << std::endl;
        return (1);
      }
      auto stream = res.unwrap();
      if (stream.is_some()) {
        auto tmp = Client(*stream);
        arr.push_back(tmp);
      }
    }

    auto element = arr.begin();
    while (element != arr.end()) {
      auto ret = element->recieveRequest();
      if (ret.is_error()) {
        std::cerr << ret.get_error() << std::endl;
        return 1;
      }
      if (*ret == false) {
        element++;
      } else {
        std::cout << element->getStream().getBuffer() << std::endl;
        std::cout << "=================================" << std::endl;
        std::cout << element->getRequest() << std::endl;
      }
    }
  }
  return (0);
}
