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
  std::cout << "Send an HTTP request using: curl http://127.0.0.1:" << PORT;
  while (1) {
    if (Networking::update_fd_status() == 0) {
      continue;
    }
    if (lis->getFdStatus() == POLLIN) {
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
    while (1) {
      if (element == arr.end()) {
        break;
      }
      auto res = element->recieveRequest();
      if (res.is_error()) {
        std::cerr << res.get_error() << std::endl;
        
      }
      if (*res == true) {
        element++;
      } else {
        std::cout << element->getRequest() << std::endl;
      }
    }
  }
}

/*

int main() {
  // Create server socket
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd == -1) {
    std::cerr << "Failed to create socket" << std::endl;
    return 1;
  }

  // Set socket options to reuse address
  int opt = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    std::cerr << "setsockopt failed" << std::endl;
    return 1;
  }

  // Bind socket to port
  struct sockaddr_in server_addr;
  std::memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
{ std::cerr << "Bind failed" << std::endl; return 1;
  }

  // Listen for connections
  if (listen(server_fd, 1) < 0) {
    std::cerr << "Listen failed" << std::endl;
    return 1;
  }

  std::cout << "Server listening on port " << PORT << std::endl;
  std::cout << "Send an HTTP request using: curl http://127.0.0.1:" << PORT <<
"/" << std::endl;

  // Accept connection
  struct sockaddr_in client_addr;
  socklen_t client_addr_len = sizeof(client_addr);
  int client_fd = accept(server_fd, (struct sockaddr *)&client_addr,
&client_addr_len); if (client_fd < 0) { std::cerr << "Accept failed" <<
std::endl; return 1;
  }

  std::cout << "\nClient connected!" << std::endl;

  // Read HTTP request
  char buffer[BUFFER_SIZE];
  std::memset(buffer, 0, BUFFER_SIZE);
  ssize_t bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);
  if (bytes_read < 0) {
    std::cerr << "Read failed" << std::endl;
    close(client_fd);
    close(server_fd);
    return 1;
  }

  std::cout << "\n=== Raw HTTP Request ===" << std::endl;
  std::cout << buffer << std::endl;
  std::cout << "=== End of Request ===" << std::endl;

  // Parse HTTP request
  auto maybe_request = Request::parse(buffer);
  if (maybe_request.is_none()) {
    std::cerr << "\nFailed to parse HTTP request!" << std::endl;
    close(client_fd);
    close(server_fd);
    return 1;
  }

  // Get parsed request and display it
  Request req = maybe_request.unwrap();
  std::cout << "\n=== Parsed Request ===" << std::endl;
  std::cout << req << std::endl;

  // Send simple response
  const char *response =
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/plain\r\n"
    "Content-Length: 13\r\n"
    "Connection: close\r\n"
    "\r\n"
    "Hello, World!";

  write(client_fd, response, std::strlen(response));

  // Cleanup
  close(client_fd);
  close(server_fd);

  return 0;
}
*/
