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
    // std::cout << "update_fd_status == 0 :: 31" << std::endl;
    if (Networking::update_fd_status() == 0) {
      sleep(1);
      continue;
    }
    if ((*lis).getFdStatus() & POLLIN) {
      auto res = Stream::accept(*lis);
      // std::cout << "after accept loop :: 37" << std::endl;
      if (res.is_error()) {
        std::cerr << res.get_error() << std::endl;
        return (1);
      }
      auto stream = res.unwrap();
      if (stream.is_some()) {
        // std::cout << "stream is some :: 45 fd = " << stream.unwrap().getFd() << std::endl;
        // std::cout << "stream is some :: 45 pl_index = " << stream.unwrap().pl_index << std::endl;
        
        arr.push_back(*stream);
      }
      // std::cout << "Creating stream :: 45" << std::endl;
      if (Networking::update_fd_status() == 0) {
      sleep(1);
      continue;
       }
    }

    auto element = arr.begin();
    while (element != arr.end()) {
      // std::cout << "In element loop :: 49" << std::endl;

      if (!element->isResponseReady()) {
        // std::cout << "here\n";
        auto ret = element->recieveRequest();
        // std::cout << "After recieveRequest :: 54" << std::endl;

        if (ret.is_error()) {
          std::cerr << ret.get_error() << std::endl;
          element->close();
          element = arr.erase(element);
          continue;
        }

        if (*ret == false) {
          // std::cout << "if (*ret == false) : 59" << std::endl;
          element++;
          continue;
        }
        // std::cout << element->getRequest() << std::endl;
        auto response_ret = element->setResponse();
        if (response_ret.is_error()) {
          std::cerr << response_ret.get_error() << std::endl;
          element->close();
          element = arr.erase(element);
          continue;
        }
      }

      if (element->isResponseReady()) {
        // std::cout << "in main before send response :: 80" << std::endl; 
        auto send_ret = element->sendResponse();
        if (send_ret.is_error()) {
          std::cerr << send_ret.get_error() << std::endl;
          element->close();
          element = arr.erase(element);
          continue;
        }

        if (element->isResponseFullySent()) {
          element->close();
          element = arr.erase(element);
          continue;
        }
      }

      element++;
    }
    // std::cout << "breaks out of the loop :: 91" << std::endl;
  }
  return (0);
}
