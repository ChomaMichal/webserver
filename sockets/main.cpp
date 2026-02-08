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

const int PORT = 2222;

void sendOnPort(void) {
  {
    const char *HOST = "127.0.0.1";
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
              << std::endl;

    sleep(10);
    close(sock);
    exit(0);
  }
}

int main() {
  Networking::init();

  auto lis = Listener::connect(2222);
  if (lis.is_error()) {
    std::cerr << "Listener error: " << lis.get_error() << std::endl;
    return 1;
  }

  int lis_fd = (*lis).getFd();
  std::cout << "Listener on port " << PORT << ", fd = " << lis_fd << std::endl;

  pid_t pid = fork();

  if (pid == 0) {
    sendOnPort();
  }

  sleep(1);
  std::cout << "Parent: Entering accept loop, lis_fd=" << lis_fd << std::endl;
  
  while (1) {
    int poll_result = Networking::update_fd_status();
    if (poll_result == -1) {
      std::cout << "DEBUG: poll ERROR: " << strerror(errno) << std::endl;
    } else {
      std::cout << "DEBUG: poll returned " << poll_result << std::endl;
    }
    usleep(100000); // 100ms to prevent CPU spinning
    auto res = (*lis).accept();
    std::cout << "DEBUG: accept() returned, checking for client..." << std::endl;
    if (res.is_error()) {
      std::cerr << res.get_error();
      return (1);
    } else if ((*res).is_some()) {
      std::cout << "got a client" << std::endl;
    }
    for (int i = 0; i < MAX_STREAMS; i++) {
      Stream &stream = Networking::getPrealocStream()[i];
      if (stream.getFd() == -1 || stream.getFd() == (*lis).getFd()) {
        continue;
      }
      short status = stream.getFdStatus();
      // Read any pending data before closing
      if (status & (POLLIN | POLLHUP)) {
        std::cout << "Reading from fd=" << stream.getFd()
                  << " (status=" << status << ")" << std::endl;
        auto hehe = stream.read();
        if (!hehe.is_error()) {
          stream.printBuffer();
          std::cout << std::endl;
        }
      }
      // Close after hangup
      if (status & POLLHUP) {
        std::cout << "Client disconnected: fd=" << stream.getFd() << std::endl;
        close(stream.getFd());
        continue;
      }
    }
  }

  return 0;
}
