#include "Listener.hpp"
#include "Networking.hpp"
#include "Stream.hpp"

#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void sendOnPort(void) {
  {
    const char *HOST = "127.0.0.1";
    const int PORT = 8080;
    const char *MESSAGE = "Hello, hard-coded world!\n";

    size_t msglen = std::strlen(MESSAGE);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
      std::cerr << "socket() failed: " << strerror(errno) << "\n";
      exit(1);
    }

    struct sockaddr_in srv;
    std::memset(&srv, 0, sizeof(srv));
    srv.sin_family = AF_INET;
    srv.sin_port = htons(static_cast<uint16_t>(PORT));
    if (inet_pton(AF_INET, HOST, &srv.sin_addr) != 1) {
      std::cerr << "inet_pton() failed for " << HOST << "\n";
      close(sock);
      exit(1);
    }

    if (connect(sock, reinterpret_cast<struct sockaddr *>(&srv), sizeof(srv)) ==
        -1) {
      std::cerr << "connect() failed: " << strerror(errno) << "\n";
      close(sock);
      exit(1);
    }

    ssize_t total_sent = 0;
    while (static_cast<size_t>(total_sent) < msglen) {
      ssize_t n = send(sock, MESSAGE + total_sent, msglen - total_sent, 0);
      if (n == -1) {
        std::cerr << "send() failed: " << strerror(errno) << "\n";
        close(sock);
        exit(1);
      }
      total_sent += n;
    }

    std::cout << "Sent " << total_sent << " bytes to " << HOST << ":" << PORT
              << "\n";

    close(sock);
    exit(0);
  }
}

int main() {
  int pid = fork();
  if (pid == 0)
    sendOnPort();

  std::cerr << "Waiting for connection..." << std::endl;
  auto lis = Listener::connect(8080);
  if (lis.is_error()) {
    std::cerr << "Connect error: " << lis.get_error() << std::endl;
    return (1);
  }
  
  std::cerr << "Listening on port 8080" << std::endl;
  
  // Poll multiple times to wait for connection
  for (int i = 0; i < 20; i++) {
    Networking::update_fd_status();
    auto err = (*lis).accept();
    if (err.is_error()) {
      std::cerr << "Accept error: " << err.get_error() << std::endl;
      return (1);
    }

    if (!(*err).is_none()) {
      std::cerr << "Got connection!" << std::endl;
      auto stream = (*err).unwrap();
      stream.read();
      std::cerr << "Received: " << std::endl;
      stream.printBuffer();
      break;
    }
    usleep(100000); // 100ms
  }
  
  std::cerr << "Waiting for child process..." << std::endl;
  waitpid(pid, NULL, 0);
  std::cerr << "Done!" << std::endl;
  return 0;
}
